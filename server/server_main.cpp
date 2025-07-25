#include "json_loader.h"
#include "bcd2ascii.h"
#include "logger.h"
#include "pgw_sessions.h"
#include "http_server.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <string>
#include <chrono>
#include <csignal>


// Глобальные переменные для graceful shutdown
std::atomic<bool> server_running{true};
HTTPServer* http_server_ptr = nullptr;

void signal_handler(int signum) {
    Logger::get()->info("Received signal {}, initiating shutdown", signum);
    server_running.store(false);
    if (http_server_ptr) {
        http_server_ptr->stop();
    }
}

int main() {
    // Установка обработчика сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Загрузка JSON конфигурации
    json_loader jsonLoader;
    jsonLoader.load("config/server_config.json");

    // Инициализация логгера
#ifdef NDEBUG
    Logger::init(jsonLoader.log_file, jsonLoader.log_level);
    CDRLogger::init(jsonLoader.cdr_file);
    Logger::get()->info("Server prepared to start in RELEASE mode");
#else
    Logger::init("/home/diminas/CLionProjects/pgw_emulator/server/logs/pgw.log", "info");
    CDRLogger::init("/home/diminas/CLionProjects/pgw_emulator/server/logs/cdr.csv");
    Logger::get()->info("Server prepared to start in DEBUG mode");
#endif

    const int SERVER_PORT = jsonLoader.udp_port;
    const int BUFFER_SIZE = jsonLoader.buffer_size;
    const int MAX_EVENTS = jsonLoader.max_events;
    const int HTTP_PORT = jsonLoader.http_port;
    const int SESSION_TIMEOUT = jsonLoader.session_timeout_sec;

    // Инициализация менеджера сессий с длительностью жизни в секундах
    SessionManager session_manager(SESSION_TIMEOUT);

    // Инициализация и запуск HTTP сервера
    HTTPServer http_server(&session_manager, HTTP_PORT);
    http_server_ptr = &http_server;

    // Настройка параметров graceful shutdown
    // Удалять по batch_size сессии каждые rep секунды
    http_server.set_shutdown_params(std::chrono::seconds(jsonLoader.graceful_shutdown_rate_sec), jsonLoader.graceful_shutdown_batch_size);

    http_server.start();

    // Создание UDP сокета
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        http_server.stop();
        return 1;
    }

    // Установка сокета в неблокирующий режим
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl O_NONBLOCK");
        close(sockfd);
        http_server.stop();
        return 1;
    }

    // Привязка к адресу
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, jsonLoader.udp_ip.c_str(), &server_addr.sin_addr) != 1) {

        std::cerr << "Ошибка: некорректный IP-адрес в конфиге: " << jsonLoader.udp_ip << std::endl;
        Logger::get()->error("main: incorrect ip address in config");
        return 1; // Завершение работы
    }


    if (bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        http_server.stop();
        return 1;
    }

    // Создание epoll инстанса
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        close(sockfd);
        http_server.stop();
        return 1;
    }

    // Добавление сокета в epoll
    epoll_event event{};
    event.events = EPOLLIN | EPOLLET; // Edge-triggered mode
    event.data.fd = sockfd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
        perror("epoll_ctl ADD");
        close(sockfd);
        close(epoll_fd);
        http_server.stop();
        return 1;
    }

    std::cout << "UDP сервер с epoll запущен на порту " << SERVER_PORT << std::endl;
    std::cout << "HTTP API сервер запущен на порту " << HTTP_PORT << std::endl;
    Logger::get()->info("PGW UDP server started with epoll on port {}", SERVER_PORT);

    epoll_event events[MAX_EVENTS];

    // Переменные для таймера очистки сессий
    auto last_cleanup = std::chrono::steady_clock::now();
    const auto cleanup_interval = std::chrono::seconds(5); // Очистка каждые 5 секунд

    // Основной цикл сервера
    while (server_running.load() && !http_server.is_shutdown_requested()) {
        // Блокируем на epoll, ждём любого события на наших фд
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // Таймаут 1 секунда

        if (nfds < 0) {
            if (errno == EINTR)
                continue;     // если прервано сигналом, просто пересадить
            perror("epoll_wait");
            break;
        }

        // Периодическая очистка просроченных сессий
        auto now = std::chrono::steady_clock::now();
        if (now - last_cleanup >= cleanup_interval) {
            session_manager.cleanup_expired_sessions();
            session_manager.print_active_sessions();
            last_cleanup = now;
        }

        for (int i = 0; i < nfds; ++i) {
            // Проверка, что это событие на нашем UDP‑сокете
            if ((events[i].events & EPOLLIN) && events[i].data.fd == sockfd) {
                // Edge‑triggered: читение до EAGAIN/EWOULDBLOCK
                while (true) {
                    uint8_t buffer[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);

                    ssize_t received = recvfrom(
                            sockfd,
                            buffer, sizeof(buffer),
                            0,
                            (sockaddr*)&client_addr, &client_len
                    );

                    if (received < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // больше данных нет - выход из inner‑loop
                            break;
                        } else {
                            perror("recvfrom");
                            break;
                        }
                    }
                    if (received == 0) {
                        // UDP‑сокет никогда не даст 0‑байтов, но на всякий случай
                        break;
                    }

                    // Преобразование raw‑буфер в вектор
                    std::vector<uint8_t> bcd_data(buffer, buffer + received);
                    // Конвертация BCD -> ASCII IMSI
                    std::string imsi = bcd2ascii(bcd_data);

                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                    uint16_t client_port = ntohs(client_addr.sin_port);
#ifdef NDEBUG
#else
                    std::cout << "Получен IMSI: " << imsi
                              << " от " << client_ip
                              << ":" << client_port << "\n";
#endif
                    Logger::get()->info("Received IMSI {} from {}:{}", imsi, client_ip, client_port);

                    std::string response;

                    // Проверка IMSI в чёрном списке
                    if (!check_imsi(imsi, jsonLoader.blacklist)) {
                        response = "rejected";
                        Logger::get()->info("IMSI {} rejected (blacklisted or invalid)", imsi);
                    } else {
                        // IMSI валиден, проверка сессии
                        if (session_manager.session_exists(imsi)) {
                            // Если сессия уже существует, обновляется активность для продолжения существования сессии
                            session_manager.update_activity(imsi);
                            response = "active";
                            Logger::get()->info("IMSI {} - session already active", imsi);
                        } else {
                            // Создание новой сессии
                            session_manager.create_session(imsi, client_ip, client_port);
                            response = "created";
                            Logger::get()->info("IMSI {} - new session created", imsi);
                        }
                    }

                    // Отправляем ответ
                    ssize_t sent = sendto(
                            sockfd,
                            response.c_str(), response.size(),
                            0,
                            (sockaddr*)&client_addr, client_len
                    );
                    if (sent < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            std::cout << "Send buffer full for " << imsi << "\n";
                        } else {
                            perror("sendto");
                        }
                    } else {
                        std::cout << "Ответ отправлен: " << response
                                  << " для IMSI: " << imsi << "\n";
                        Logger::get()->info("Response sent: {} for IMSI {}", response, imsi);
                    }
                }
            }
        }
    }

    Logger::get()->info("Main server loop ended, cleaning up...");

    // Очистка ресурсов
    http_server.stop();
    close(sockfd);
    close(epoll_fd);

    Logger::get()->info("Server shutdown complete");
    return 0;
}