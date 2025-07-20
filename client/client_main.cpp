#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

// Функция для конвертации ASCII IMSI в BCD
std::vector<uint8_t> ascii_to_bcd(const std::string& imsi) {
    std::vector<uint8_t> bcd;
    std::string padded_imsi = imsi;

    // Добавляем 0 в начало, если длина нечетная
    if (padded_imsi.length() % 2 != 0) {
        padded_imsi = "0" + padded_imsi;
    }

    for (size_t i = 0; i < padded_imsi.length(); i += 2) {
        uint8_t high = padded_imsi[i] - '0';
        uint8_t low = padded_imsi[i + 1] - '0';
        bcd.push_back((high << 4) | low);
    }

    return bcd;
}

int main(int argc, char* argv[]) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    std::string imsi = "250456789012345"; // IMSI по умолчанию

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
    std::vector<uint8_t> bcd_data = ascii_to_bcd(imsi);

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

    close(sockfd);
    return 0;
}