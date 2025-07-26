#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include "ascii2bcd.h"
#include "logger.h"
#include "client_json_loader.h"


int main(int argc, char* argv[]) {
    client_json_loader jsonLoader;
    jsonLoader.load("config/client_config.json");
#ifdef NDEBUG
    Logger::init(jsonLoader.log_file, jsonLoader.log_level);
    Logger::get()->info("Client prepared to start in RELEASE mode");
#else
    Logger::init("/home/diminas/CLionProjects/pgw_emulator/client/logs/client.log", "info");
    Logger::get()->info("Client prepared to start in DEBUG mode");
#endif

    std::string server_ip = jsonLoader.server_ip;
    int server_port = jsonLoader.server_port;
    std::string imsi;
    if (argc == 2){
        imsi = argv[1];
    }else{
        imsi = "250123456"; // IMSI по умолчанию
    }

    Logger::get()->info("Подключение к серверу {}:{}", server_ip, server_port);

    // Простой парсинг аргументов
    if (argc >= 2) {
        imsi = argv[1];
    }
    if (argc >= 3) {
        server_ip = argv[2];
    }
    if (argc >= 4) {
        server_port = std::stoi(argv[3]);
    }

    std::cout << "Подключение к серверу: " << server_ip << ":" << server_port << std::endl;
    std::cout << "Отправляем IMSI: " << imsi << std::endl;

    // Создание сокета
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Неверный IP адрес: " << server_ip << std::endl;
        close(sockfd);
        return 1;
    }

    // Конвертация IMSI в BCD
    std::vector<uint8_t> bcd_data = ascii2bcd(imsi);

    std::cout << "BCD данные: ";
    for (uint8_t byte : bcd_data) {
        printf("%02X ", byte);
    }
    std::cout << std::endl;

    // Отправка данных
    ssize_t sent = sendto(sockfd, bcd_data.data(), bcd_data.size(), 0,
                          (sockaddr*)&server_addr, sizeof(server_addr));

    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    }

    std::cout << "Отправлено " << sent << " байт" << std::endl;

    // Получение ответа
    char response[256];
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);

    ssize_t received = recvfrom(sockfd, response, sizeof(response) - 1, 0,
                                (sockaddr*)&from_addr, &from_len);

    if (received < 0) {
        perror("recvfrom");
        close(sockfd);
        return 1;
    }

    response[received] = '\0'; // Null-terminate строку

    std::cout << "Получен ответ: '" << response << "'" << std::endl;
    std::cout << "Размер ответа: " << received << " байт" << std::endl;
    spdlog::shutdown();
    close(sockfd);
    return 0;
}