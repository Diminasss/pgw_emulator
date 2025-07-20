#include "include/json_loader.h"
#include "logger.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <string>


using std::cout;
using std::endl;

// Простой парсер BCD → ASCII IMSI
std::string bcd_to_ascii(const uint8_t* bcd, size_t len) {
    std::string result;
    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = bcd[i];
        uint8_t low = byte & 0x0F;
        uint8_t high = (byte & 0xF0) >> 4;

        if (low <= 9)
            result += ('0' + low);
        else
            break;

        if (high <= 9)
            result += ('0' + high);
        else
            break;
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
    Logger::init("/home/diminas/CLionProjects/pgw_emulator/server/logs/pgw.log", "info");
    Logger::get()->info("Server prepared to start in DEBUG mode");
#endif

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
        // Ожидание событий
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (nfds < 0) {
            if (errno == EINTR) continue; // Прерывание сигналом
            perror("epoll_wait");
            break;
        }

        // Обработка событий
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == sockfd) {
                // Обработка входящих UDP пакетов
                while (true) {
                    uint8_t buffer[BUFFER_SIZE];
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);

                    ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                                (sockaddr*)&client_addr, &client_len);

                    if (received < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Нет больше данных в edge-triggered режиме
                            break;
                        } else {
                            perror("recvfrom");
                            break;
                        }
                    }

                    if (received == 0) {
                        break;
                    }

                    // Обработка полученных данных
                    std::string imsi = bcd_to_ascii(buffer, received);
                    std::cout << "Получен IMSI: " << imsi
                              << " от " << inet_ntoa(client_addr.sin_addr)
                              << ":" << ntohs(client_addr.sin_port) << std::endl;

                    Logger::get()->info("Received IMSI: " + imsi + " from " +
                                        inet_ntoa(client_addr.sin_addr));

                    // Определение ответа
                    std::string response = handle_imsi(imsi) ? "created" : "rejected";

                    // Отправка ответа
                    ssize_t sent = sendto(sockfd, response.c_str(), response.size(), 0,
                                          (sockaddr*)&client_addr, client_len);

                    if (sent < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // Буфер отправки заполнен, можно добавить в очередь
                            std::cout << "Буфер отправки заполнен для " << imsi << std::endl;
                        } else {
                            perror("sendto");
                        }
                    } else {
                        std::cout << "Ответ отправлен: " << response
                                  << " для IMSI: " << imsi << std::endl;
                        Logger::get()->info("Response sent: " + response + " for IMSI: " + imsi);
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