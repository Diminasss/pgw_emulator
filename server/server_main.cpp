#include "include/json_loader.h"
#include "logger.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <bitset>


using std::cout;
using std::endl;

// Функция для конвертации BCD в ASCII для IMSI
std::string bcd2ascii(const std::vector<uint8_t>& bcd) {
    std::string result;
    size_t len = bcd.size();

    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = bcd[i];

        // Сначала старший ниббл
        uint8_t high = (byte & 0xF0) >> 4;
        uint8_t low  = byte & 0x0F;

        // Обработка старшего ниббла
        if (high <= 9) {
            result += static_cast<char>('0' + high);
        } else if (high == 0xF) {
            break; // padding — конец строки
        } else {
            break; // недопустимый BCD
        }

        // Обработка младшего ниббла
        if (low <= 9) {
            result += static_cast<char>('0' + low);
        } else if (low == 0xF) {
            break; // padding — конец строки
        } else {
            break; // недопустимый BCD
        }
    }

    return result;
}


bool handle_imsi(const std::string& imsi_ascii) {
    // Заглушка: одобряем IMSI, если оно начинается с "250"
    return imsi_ascii.starts_with("250");
}


int main() {
    // Загрузка JSON конфигурации
    json_loader jsonLoader;
    jsonLoader.load("config/server_config.json");

    // Инициализация логгера
#ifdef NDEBUG
    Logger::init(jsonLoader.log_file, jsonLoader.log_level);
    Logger::get()->info("Server prepared to start in RELEASE mode");
#else
    Logger::init("/home/diminas/CLionProjects/pgw_emulator/server/logs/pgw.logs", "info");
    Logger::get()->info("Server prepared to start in DEBUG mode");
#endif

//    uint8_t test_bcd[] = {0x52, 0x09, 0x91, 0x32, 0x54, 0x76, 0x98, 0x0F};
//    std::string result = bcd_to_ascii(test_bcd, 8);
//    std::cout << "Результат: " << result << std::endl; // Должно быть: 250991234567890
//    cout << "Должно быть 250991234567890" << endl;
//    if (result == "250991234567890"){
//        cout << "Верно!" << endl;
//    }
//    else{
//        cout << "НЕ верно!" << endl;
//    }

    const int SERVER_PORT = jsonLoader.udp_port;
    const int BUFFER_SIZE = jsonLoader.buffer_size;
    const int MAX_EVENTS = 10;

    // Создание UDP сокета
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Установка сокета в неблокирующий режим
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl O_NONBLOCK");
        close(sockfd);
        return 1;
    }

    // Привязка к адресу
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    // Создание epoll инстанса
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        close(sockfd);
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
        return 1;
    }

    std::cout << "UDP сервер с epoll запущен на порту " << SERVER_PORT << std::endl;
    Logger::get()->info("PGW UDP server started with epoll on port " + std::to_string(SERVER_PORT));

    epoll_event events[MAX_EVENTS];

    while (true) {
        // Блокируем на epoll, ждём любого события на наших фд
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR)
                continue;     // если прервано сигналом, просто пересадить
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            // Проверяем, что это событие на нашем UDP‑сокете
            if ((events[i].events & EPOLLIN) && events[i].data.fd == sockfd) {
                // Edge‑triggered: читаем до EAGAIN/EWOULDBLOCK
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
                            // больше данных нет — выходим из inner‑loop
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

                    // Преобразуем raw‑буфер в вектор
                    std::vector<uint8_t> bcd_data(buffer, buffer + received);
                    // Конвертируем BCD → ASCII IMSI
                    std::string imsi = bcd2ascii(bcd_data);

                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

                    std::cout << "Получен IMSI: " << imsi
                              << " от " << client_ip
                              << ":" << ntohs(client_addr.sin_port) << "\n";
                    Logger::get()->info("Received IMSI {} from {}", imsi, client_ip);

                    // Решаем, что отвечать
                    std::string response = handle_imsi(imsi)
                                           ? "created"
                                           : "rejected";

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

    // Очистка ресурсов
    close(sockfd);
    close(epoll_fd);
    return 0;
}