#pragma once

#include "logger.h"

#include <string>
#include <cstdint>
#include <chrono>
#include <unordered_map>

// Структура для хранения информации о сессии
struct PGWSession {
    std::string imsi;
    std::string client_ip;
    uint16_t client_port;
    std::chrono::steady_clock::time_point created_time;
    std::chrono::steady_clock::time_point last_activity;

    PGWSession(std::string  imsi, std::string  ip, uint16_t port);
};

// Класс для управления сессиями
class SessionManager {
private:
    std::unordered_map<std::string, PGWSession> active_sessions; // ключ = IMSI
    std::chrono::seconds session_timeout;

public:
    // Конструктор
    explicit SessionManager(int timeout_seconds = 300) : session_timeout(timeout_seconds) {}
    // Создание новой сессии
    bool create_session(const std::string& imsi, const std::string& client_ip, uint16_t client_port);
    // Проверка существования сессии
    bool session_exists(const std::string& imsi);
    // Удаление сессии
    void delete_session(const std::string& imsi);
    // Очистка просроченных сессий
    void cleanup_expired_sessions();
    // Обновление времени активности сессии
    void update_activity(const std::string& imsi);
    // Получение количества активных сессий
    size_t get_active_sessions_count() const;
    // Вывод информации о всех активных сессиях
    void print_active_sessions() const;
    // Получение списка всех IMSI (для graceful shutdown)
    std::vector<std::string> get_all_imsis() const;
    // Удаление батча сессий (возвращает количество удаленных)
    size_t remove_sessions_batch(size_t batch_size = 5);
};
