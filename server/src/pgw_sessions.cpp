#include "pgw_sessions.h"

PGWSession::PGWSession(std::string imsi, std::string ip, uint16_t port)
        : imsi(std::move(imsi)), client_ip(std::move(ip)), client_port(port),
          created_time(std::chrono::steady_clock::now()),
          last_activity(std::chrono::steady_clock::now()) {}

bool SessionManager::create_session(const std::string& imsi, const std::string& client_ip, uint16_t client_port) {
    auto it = active_sessions.find(imsi);
    if (it != active_sessions.end()) {
        // Сессия уже существует - обновляем время последней активности
        it->second.last_activity = std::chrono::steady_clock::now();
        Logger::get()->info("Session for IMSI {} already exists, updated activity time", imsi);
        return false; // Сессия уже была
    }
//        if (active_sessions.find(imsi) != active_sessions.end()) {
//            // Сессия уже существует - обновляем время последней активности
//            active_sessions[imsi].last_activity = std::chrono::steady_clock::now();
//            Logger::get()->info("Session for IMSI {} already exists, updated activity time", imsi);
//            return false; // Сессия уже была
//        }

    // Создаем новую сессию
    active_sessions.emplace(imsi, PGWSession(imsi, client_ip, client_port));
    Logger::get()->info("Created new session for IMSI {} from {}:{}", imsi, client_ip, client_port);
    return true; // Новая сессия создана
}

bool SessionManager::session_exists(const std::string& imsi) {
    return active_sessions.find(imsi) != active_sessions.end();
}

void SessionManager::delete_session(const std::string& imsi) {
    auto it = active_sessions.find(imsi);
    if (it != active_sessions.end()) {
        Logger::get()->info("Deleting session for IMSI {}", imsi);
        active_sessions.erase(it);
    }
}

void SessionManager::cleanup_expired_sessions() {
    auto now = std::chrono::steady_clock::now();
    auto it = active_sessions.begin();

    while (it != active_sessions.end()) {
        auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(
                now - it->second.last_activity);

        if (time_diff >= session_timeout) {
            Logger::get()->info("Session for IMSI {} expired (inactive for {} seconds)",
                                it->first, time_diff.count());
            it = active_sessions.erase(it);
        } else {
            ++it;
        }
    }
}

void SessionManager::update_activity(const std::string& imsi) {
    auto it = active_sessions.find(imsi);
    if (it != active_sessions.end()) {
        it->second.last_activity = std::chrono::steady_clock::now();
    }
}

size_t SessionManager::get_active_sessions_count() const {
    return active_sessions.size();
}

void SessionManager::print_active_sessions() const {
    Logger::get()->info("Active sessions: {}", active_sessions.size());
    for (const auto& [imsi, session] : active_sessions) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - session.created_time);
        Logger::get()->info("  IMSI: {} from {}:{}, active for {} seconds",
                            imsi, session.client_ip, session.client_port, duration.count());
    }
}