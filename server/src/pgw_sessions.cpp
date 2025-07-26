#include "pgw_sessions.h"
#include "cdr_logger.h"

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

        // Записываем CDR об обновлении сессии
        std::string additional_info = "from " + client_ip + ":" + std::to_string(client_port);
        CDRLogger::write_cdr(imsi, CDRAction::SESSION_UPDATE, additional_info);

        return false; // Сессия уже была
    }

    // Создаем новую сессию
    active_sessions.emplace(imsi, PGWSession(imsi, client_ip, client_port));
    Logger::get()->info("Created new session for IMSI {} from {}:{}", imsi, client_ip, client_port);

    // Записываем CDR о создании сессии
    std::string additional_info = "from " + client_ip + ":" + std::to_string(client_port);
    CDRLogger::write_cdr(imsi, CDRAction::SESSION_CREATE, additional_info);

    return true; // Новая сессия создана
}

bool SessionManager::session_exists(const std::string& imsi) {
    return active_sessions.find(imsi) != active_sessions.end();
}

// На будущее мб
/*
void SessionManager::delete_session(const std::string& imsi) {
    auto it = active_sessions.find(imsi);
    if (it != active_sessions.end()) {
        Logger::get()->info("Deleting session for IMSI {}", imsi);

        // Записываем CDR об удалении сессии
        CDRLogger::write_cdr(imsi, CDRAction::SESSION_DELETE);

        active_sessions.erase(it);
    }
}
*/
void SessionManager::cleanup_expired_sessions() {
    auto now = std::chrono::steady_clock::now();
    auto it = active_sessions.begin();

    while (it != active_sessions.end()) {
        auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(
                now - it->second.last_activity);

        if (time_diff >= session_timeout) {
            Logger::get()->info("Session for IMSI {} expired (inactive for {} seconds)",
                                       it->first, time_diff.count());

            // Записываем CDR об истечении сессии
            std::string additional_info = "timeout " + std::to_string(time_diff.count()) + "s";
            CDRLogger::write_cdr(it->first, CDRAction::SESSION_EXPIRE, additional_info);

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

// На будущее мб
/*
std::vector<std::string> SessionManager::get_all_imsis() const {
    std::vector<std::string> imsis;
    imsis.reserve(active_sessions.size());

    for (const auto& [imsi, session] : active_sessions) {
        imsis.push_back(imsi);
    }

    return imsis;
}
*/
size_t SessionManager::remove_sessions_batch(size_t batch_size) {
    if (active_sessions.empty()) {
        return 0;
    }

    size_t removed = 0;
    auto it = active_sessions.begin();

    while (it != active_sessions.end() && removed < batch_size) {
        Logger::get()->info("Graceful shutdown: removing session for IMSI {}", it->first);

        // Записываем CDR об удалении сессии при graceful shutdown
        CDRLogger::write_cdr(it->first, CDRAction::SESSION_DELETE, "graceful_shutdown");

        it = active_sessions.erase(it);
        removed++;
    }

    return removed;
}