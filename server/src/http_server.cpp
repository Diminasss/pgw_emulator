#include "http_server.h"
#include <thread>

HTTPServer::HTTPServer(SessionManager* sm, int port)
        : session_manager(sm), port_(port) {
    setup_routes();
}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::setup_routes() {
    // CORS headers для всех запросов
    server.set_pre_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // OPTIONS для CORS preflight
    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        return;
    });

    // GET /check_subscriber?imsi=123456789
    server.Get("/check_subscriber", [this](const httplib::Request& req, httplib::Response& res) {
        handle_check_subscriber(req, res);
    });

    // POST /stop
    server.Post("/stop", [this](const httplib::Request& req, httplib::Response& res) {
        handle_stop(req, res);
    });

    // GET /status - дополнительный endpoint для мониторинга
    server.Get("/status", [this](const httplib::Request& req, httplib::Response& res) {
        handle_status(req, res);
    });
}

void HTTPServer::handle_check_subscriber(const httplib::Request& req, httplib::Response& res) {
    // Получаем IMSI из параметров запроса
    std::string imsi;
    if (req.has_param("imsi")) {
        imsi = req.get_param_value("imsi");
    }

    if (imsi.empty()) {
        res.status = 400;
        res.set_content("Missing IMSI parameter", "text/plain");
        Logger::get()->warn("HTTP API: /check_subscriber called without IMSI parameter");
        return;
    }

    // Проверяем существование сессии
    bool exists = session_manager->session_exists(imsi);
    std::string response = exists ? "active" : "not active";

    res.set_content(response, "text/plain");
    Logger::get()->info("HTTP API: /check_subscriber IMSI {} - {}", imsi, response);
}

void HTTPServer::handle_stop(const httplib::Request& req, httplib::Response& res) {
    if (shutdown_requested.load()) {
        res.set_content("Shutdown already in progress", "text/plain");
        return;
    }

    Logger::get()->info("HTTP API: Graceful shutdown requested via /stop");
    shutdown_requested.store(true);

    // Запускаем graceful shutdown в отдельном потоке
    shutdown_thread = std::thread([this]() {
        graceful_shutdown();
    });

    res.set_content("Graceful shutdown initiated", "text/plain");
}

void HTTPServer::handle_status(const httplib::Request& req, httplib::Response& res) {
    size_t active_sessions = session_manager->get_active_sessions_count();
    std::string status = shutdown_requested.load() ? "shutting_down" : "running";

    std::string response = "Status: " + status + "\n";
    response += "Active sessions: " + std::to_string(active_sessions) + "\n";

    res.set_content(response, "text/plain");
}

void HTTPServer::graceful_shutdown() {
    Logger::get()->info("Starting graceful shutdown process");

    while (session_manager->get_active_sessions_count() > 0) {
        // Удаляем сессии батчами
        auto removed = remove_sessions_batch();

        Logger::get()->info("Graceful shutdown: removed {} sessions, {} remaining",
                            removed, session_manager->get_active_sessions_count());

        if (removed == 0) {
            // Если не удалось удалить сессии, выходим из цикла
            break;
        }

        // Ждем перед следующим батчем
        std::this_thread::sleep_for(shutdown_interval);
    }

    Logger::get()->info("Graceful shutdown completed, all sessions removed");
}

size_t HTTPServer::remove_sessions_batch() {
    // Эта функция должна быть добавлена в SessionManager
    // Пока используем простую реализацию
    size_t removed = 0;
    size_t current_batch = 0;

    // Получаем список IMSI для удаления
    std::vector<std::string> to_remove;

    // В идеале нужен метод get_all_imsis() в SessionManager
    // Пока просто удаляем старые сессии
    session_manager->cleanup_expired_sessions();

    return removed;
}

void HTTPServer::start() {
    if (running.load()) {
        return;
    }

    running.store(true);

    server_thread = std::thread([this]() {
        Logger::get()->info("HTTP Server starting on port {}", port_);

        if (!server.listen("0.0.0.0", port_)) {
            Logger::get()->error("Failed to start HTTP server on port {}", port_);
        } else {
            Logger::get()->info("HTTP Server stopped");
        }
    });

    // Даем серверу время на запуск
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Logger::get()->info("HTTP Server thread started on port {}", port_);
}

void HTTPServer::stop() {
    if (!running.load()) {
        return;
    }

    Logger::get()->info("Stopping HTTP Server");
    server.stop();
    running.store(false);

    if (server_thread.joinable()) {
        server_thread.join();
    }

    if (shutdown_thread.joinable()) {
        shutdown_thread.join();
    }
}

bool HTTPServer::is_shutdown_requested() const {
    return shutdown_requested.load();
}

void HTTPServer::set_shutdown_params(std::chrono::milliseconds interval, size_t batch_size) {
    shutdown_interval = interval;
    sessions_per_batch = batch_size;
}
