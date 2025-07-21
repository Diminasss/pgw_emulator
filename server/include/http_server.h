#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <httplib.h>
#include <thread>
#include <atomic>
#include <chrono>
#include "pgw_sessions.h"
#include "logger.h"

class HTTPServer {
private:
    httplib::Server server;
    std::thread server_thread;
    std::atomic<bool> running{false};
    std::atomic<bool> shutdown_requested{false};
    SessionManager* session_manager;

    // Параметры graceful shutdown
    std::thread shutdown_thread;
    std::chrono::milliseconds shutdown_interval{1000}; // удаление сессий каждую секунду
    size_t sessions_per_batch{5}; // удалять по 5 сессий за раз

    int port_;

    // Приватные методы
    void setup_routes();
    void handle_check_subscriber(const httplib::Request& req, httplib::Response& res);
    void handle_stop(const httplib::Request& req, httplib::Response& res);
    void handle_status(const httplib::Request& req, httplib::Response& res);
    void graceful_shutdown();
    size_t remove_sessions_batch();

public:
    explicit HTTPServer(SessionManager* sm, int port = 8080);
    ~HTTPServer();

    void start();
    void stop();
    bool is_shutdown_requested() const;
    void set_shutdown_params(std::chrono::milliseconds interval, size_t batch_size);
};

#endif // HTTP_SERVER_H