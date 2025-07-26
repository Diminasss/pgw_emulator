# Mini-PGW Emulator

## Структура проекта
```
├── client                                            # Папка с клиентом
│   ├── client_main.cpp                         # main функция клиента
│   ├── CMakeLists.txt                          # Инструкции для сборки клиента
│   ├── config                                  # Папка с конфигурационным файлом клиента
│   │   └── client_config.json            # Конфигурационный файл клиента
│   ├── include                                 # Директория включаемых файлов для клиента
│   │   ├── ascii2bcd.h                   # Конвертация ascii в bcd ивключаемый файл
│   │   ├── client_json_loader.h          # Подгрузка json включаемый файл
│   │   └── client_logger.h               # Логгер клиента включаемый файл
│   ├── logs                                    # Логи клиента
│   │   └── client.log                    # Лог клиента
│   ├── py_client                               # HTTP клиенты на Python
│   │   ├── http_check_subscriber.py      # Клиент для вызова /check_subscriber
│   │   ├── http_status.py                # Клиент для вызовва /status
│   │   ├── http_stop.py                  # Клиент для вызова /stop
│   │   └── test_config.py                # Файл для конфигурации HTTP клиентов 
│   └── src                                     # Директория исполняемых файлов клиента
│       ├── ascii2bcd.cpp                       # Конвертация ascii в bcd реализация
│       ├── client_json_loader.cpp              # Подгрузка json реализация
│       └── client_logger.cpp                   # Логгер клиента реализация
├── CMakeLists.txt                                    # Инструкции для сборки всего проекта
├── README.md                                         # Этот файл
├── requirements.txt                                  # Необходимые зависимости для Python
└── server                                            # Папка с сервером
    ├── CMakeLists.txt                                # Инструкции для сборки сервера
    ├── config                                        # Папка с конфигурационным файлом сервера
    │   └── server_config.json                  # Конфигурационный файл сервера
    ├── include                                       # Директория включаемых файлов для сервера
    │   ├── bcd2ascii.h                         # Конвертация bcd в ascii и проверка imsi включаемый файл
    │   ├── cdr_logger.h                        # Логгер для ведения CDR журнала включаемый файл
    │   ├── http_server.h                       # HTTP сервер, работающий дополнительно с UDP включаемый файл
    │   ├── json_loader.h                       # Загрузчик json для сервера включаемый файл
    │   ├── logger.h                            # Логгер сервера включаемый файл
    │   └── pgw_sessions.h                      # Управление сессиями включаемый файл
    ├── logs                                          # Папка с логами сервера
    │   ├── cdr.csv                             # CDR журнал
    │   └── pgw.log                             # Лог сервера
    ├── server_main.cpp                               # main функция сервера
    ├── src                                           # Директория исполняемых файлов сервера
    │   ├── bcd2ascii.cpp                       # Конвертация bcd в ascii и проверка imsi реализация
    │   ├── cdr_logger.cpp                      # Логгер для ведения CDR журнала реализация
    │   ├── http_server.cpp                     # HTTP сервер, работающий дополнительно с UDP реализация
    │   ├── json_loader.cpp                     # Загрузчик json для сервера реализация
    │   ├── logger.cpp                          # Логгер сервера реализация
    │   └── pgw_sessions.cpp                    # Управление сессиями реализация
    └── test                                          # Папка с тестами для сервера
        ├── CMakeLists.txt                            # Инструкции при сборке тестов
        ├── py_http_tests                             # HTTP юнит тесты
        │   ├── config                          # Конфигурационные файлы при работе юнит тестов Python 
        │   │   ├── client_config.json    # Для клиента
        │   │   └── server_config.json    # Для сервера
        │   ├── http_test_check_subscriber.py   # Тест проверки /check_subscriber в формате unit test
        │   ├── http_test_status.py             # Тест проверки /status в формате unit test
        │   └── http_test_stop.py               # Тест проверки /stop в формате unit test
        ├── test_main.cpp                             # main функция для тестирования
        └── tests                                     # Тесты для сервера
            ├── test_bcd2ascii.cpp 
            ├── test_json_loader.cpp
            └── test_cdr_logger.cpp
```
## Описание проекта

Мини-PGW выполняет следующие задачи:
- Приём UDP-запросов с IMSI в BCD-кодировке.
- Создание и отслеживание сессий абонентов (класс `SessionManager`) с учётом таймаута.
- Обработка чёрного списка IMSI (отклонение запросов для заблокированных IMSI).
- UDP сервер может вернуть created - сессия создана, rejected - сессия отклонена, active - сессия уже существует, но была обновлена (обновлён таймер) (добавлено такое поведение, так как в задании об этом не было сказано)
- Ведение CDR-журнала (`cdr.csv`) с записями о создании, обновлении и удалении сессий.
- Предоставление HTTP API:
    - `/check_subscriber?imsi=<imsi>` — проверка активности сессии (`active`/`not active`).
    - `/status` — статус сервера (количество активных сессий и т.д.).
    - `/stop` — плавное завершение работы (graceful shutdown), удаление сессий батчами.
- Логирование ключевых событий с помощью `spdlog` (файлы логов: `pgw.log` и `cdr.csv`).

Клиентское приложение (`client`) выполняет:
- Загрузку настроек из `config/client_config.json`.
- Конвертацию IMSI в BCD и отправку UDP-пакета серверу.
- Приём текстового ответа ('created' или 'rejected') и вывод на экран.
- Логирование операций в файл `client.log`.

## Установка и сборка

1. Требуется **C++23** и **CMake 3.31+** на Linux.
2. Склонировать репозиторий и перейти в папку проекта.
3. Создать директорию сборки:
    ```bash
    mkdir build && cd build
    ```
4. Запустить CMake и собрать проект:
    ```bash
    cmake ..
    make
    ```
5. Конфигурационные JSON-файлы автоматически копируются в папку сборки.
## Запуск
**Сервер**: выполнить ./server. Сервер читает параметры из config/server_config.json.

**Клиент**: выполнить ./client [IMSI] [server_ip] [server_port]. 

## Аргументы:
- IMSI (строка цифр), IP и порт сервера (по умолчанию берутся из config/client_config.json).
- HTTP API: по умолчанию сервер слушает на порту, указанном в конфиге (например, 8080). Запросы:
- GET http://<server_ip>:<http_port>/check_subscriber?imsi=<imsi>
- GET http://<server_ip>:<http_port>/status
- POST http://<server_ip>:<http_port>/stop

Python HTTP-тесты: в папке server/test/py_http_tests. Установить зависимости:
```bash
pip3 install -r requirements.txt
```
и запускать скрипты: 
```
python3 http_test_status.py, http_test_check_subscriber.py, http_test_stop.py
```
предварительно указав путь к исполняемым файлам.
Также есть возможность работать из python файла в качестве клиента. Они находятся в папке client/py_client. Для их работы необходимо отдельно запустить сервер.
## Примеры конфигурационных файлов
server_config.json:
```
{
  "udp_ip": "0.0.0.0",
  "udp_port": 9000,
  "buffer_size": 1024,
  "session_timeout_sec": 60,
  "cdr_file": "cdr.csv",
  "http_port": 8080,
  "graceful_shutdown_batch_size": 3,
  "graceful_shutdown_rate_sec": 10,
  "log_file": "pgw.log",
  "log_level": "info",
  "max_events": 10,
  "blacklist": [
    "001010123456789",
    "001010000000001",
    "2501234565"
  ]
}
```
client_config.json:
```
{
  "server_ip": "127.0.0.1",
  "server_port": 9000,
  "log_file": "client.log",
  "log_level": "info"
}
```
## Используемые технологии
- C++23, CMake.
- Библиотеки: spdlog (логирование), nlohmann/json (JSON), cpp-httplib (HTTP).
- Google Test для модульного тестирования.
- Python 3 и requests (для HTTP-клиентов и тестов).