#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <fstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <mysql/mysql.h>

#define SERVER_TCP_PORT 9000

// === Logger ===
std::mutex logFileMutex;

void saveLogToFile(const std::string& logLine) {
    std::lock_guard<std::mutex> lock(logFileMutex);
    std::ofstream file("log.txt", std::ios::app);
    if (file.is_open()) {
        file << logLine << std::endl;
    }
}

// === Messages File ===
std::mutex messageFileMutex;

void saveMessageToFile(const std::string& sender, const std::string& receiver, const std::string& message) {
    std::lock_guard<std::mutex> lock(messageFileMutex);
    std::ofstream file("messages.txt", std::ios::app);
    if (file.is_open()) {
        file << sender << " -> " << receiver << " : " << message << "\n";
    }
}

// === Client Info ===
struct ClientInfo {
    int sockfd;
    sockaddr_in addr;
    bool isAdmin;
};

std::map<std::string, ClientInfo> clients;  // username -> ClientInfo
std::mutex clients_mutex;

MYSQL* conn;

// === DB helpers ===

std::string escapeString(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '\'') output += "''";
        else output += c;
    }
    return output;
}

bool connectToDB() {
    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL init failed\n";
        return false;
    }
    if (!mysql_real_connect(conn, "localhost", "root", "root", nullptr, 0, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(conn) << "\n";
        return false;
    }
    mysql_query(conn, "CREATE DATABASE IF NOT EXISTS chatdb");
    mysql_select_db(conn, "chatdb");

    const char* createUsers = "CREATE TABLE IF NOT EXISTS users ("
                              "username VARCHAR(100) PRIMARY KEY,"
                              "password VARCHAR(100),"
                              "is_admin TINYINT(1) DEFAULT 0)";

    const char* createMessages = "CREATE TABLE IF NOT EXISTS messages ("
                                 "id INT AUTO_INCREMENT PRIMARY KEY,"
                                 "sender VARCHAR(100),"
                                 "receiver VARCHAR(100),"
                                 "text TEXT)";

    mysql_query(conn, createUsers);
    mysql_query(conn, createMessages);

    return true;
}

// Структура результата проверки логина
struct UserInfo {
    bool valid;
    bool isAdmin;
};

UserInfo validateLogin(const std::string& user, const std::string& pass) {
    std::string query = "SELECT is_admin FROM users WHERE username='" + escapeString(user) +
                        "' AND password='" + escapeString(pass) + "'";
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB select error: " << mysql_error(conn) << "\n";
        return {false, false};
    }
    MYSQL_RES* res = mysql_store_result(conn);
    bool valid = mysql_num_rows(res) > 0;
    bool isAdmin = false;
    if (valid) {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && row[0]) {
            isAdmin = (std::stoi(row[0]) != 0);
        }
    }
    mysql_free_result(res);
    return {valid, isAdmin};
}

bool registerUser(const std::string& user, const std::string& pass) {
    std::string query = "INSERT INTO users(username, password) VALUES('" + escapeString(user) + "', '" + escapeString(pass) + "')";
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB insert error: " << mysql_error(conn) << "\n";
        return false;
    }
    return true;
}

void saveMessageToDB(const std::string& sender, const std::string& receiver, const std::string& text) {
    std::string query = "INSERT INTO messages(sender, receiver, text) VALUES('" +
                        escapeString(sender) + "', '" + escapeString(receiver) + "', '" + escapeString(text) + "')";
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB insert error: " << mysql_error(conn) << "\n";
    }
}

// === Message handling ===

void handlePacketTCP(int clientSock, const std::string& data, sockaddr_in clientAddr) {
    // Получаем имя пользователя и права админа по сокету
    std::string username;
    bool isAdmin = false;
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto& pair : clients) {
            if (pair.second.sockfd == clientSock) {
                username = pair.first;
                isAdmin = pair.second.isAdmin;
                break;
            }
        }
    }

    if (data.rfind("REGISTER:", 0) == 0) {
        auto body = data.substr(9);
        auto sep = body.find(':');
        if (sep == std::string::npos) return;
        std::string user = body.substr(0, sep);
        std::string pass = body.substr(sep + 1);

        bool success = registerUser(user, pass);
        std::string reply = success ? "REGISTER_OK\n" : "REGISTER_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);

        saveLogToFile("REGISTER " + user + " : " + (success ? "SUCCESS" : "FAIL"));
    }
    else if (data.rfind("LOGIN:", 0) == 0) {
        auto body = data.substr(6);
        auto sep = body.find(':');
        if (sep == std::string::npos) return;
        std::string user = body.substr(0, sep);
        std::string pass = body.substr(sep + 1);

        UserInfo info = validateLogin(user, pass);
        if (info.valid) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients[user] = ClientInfo{clientSock, clientAddr, info.isAdmin};
            }
            std::string reply = "LOGIN_OK\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            saveLogToFile("LOGIN " + user + " : SUCCESS");
        } else {
            std::string reply = "LOGIN_FAIL\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            saveLogToFile("LOGIN " + user + " : FAIL");
        }
    }
    else if (data.rfind("MSG:", 0) == 0) {
        size_t pos1 = data.find(':', 4);
        if (pos1 == std::string::npos) return;
        std::string receiver = data.substr(4, pos1 - 4);
        std::string message = data.substr(pos1 + 1);

        if (username.empty()) return; // Неавторизованный клиент не может отправлять сообщения

        saveMessageToDB(username, receiver, message);
        saveMessageToFile(username, receiver, message);

        std::lock_guard<std::mutex> lock(clients_mutex);
        if (clients.count(receiver)) {
            std::string out = "FROM:" + username + ":" + message + "\n";
            send(clients[receiver].sockfd, out.c_str(), out.size(), 0);
        }
    }
    else if (data == "GET_USERS") {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::string usersList = "USERS:";
        for (const auto& pair : clients) {
            usersList += pair.first + ",";
        }
        if (!clients.empty())
            usersList.pop_back(); // убрать последнюю запятую
        usersList += "\n";
        send(clientSock, usersList.c_str(), usersList.size(), 0);
    }
    else if (data == "GET_LOGS") {
        if (!isAdmin) {
            std::string reply = "ERROR: Not authorized\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            return;
        }

        std::ifstream logFile("log.txt");
        if (!logFile.is_open()) {
            std::string reply = "LOGS:ERROR_OPENING_LOG\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
        } else {
            std::string logs = "LOGS:";
            std::string line;
            bool first = true;
            while (std::getline(logFile, line)) {
                if (!first) logs += "|";
                logs += line;
                first = false;
            }
            logs += "\n";
            send(clientSock, logs.c_str(), logs.size(), 0);
        }
    }
    else if (data == "GET_MESSAGES") {
        std::ifstream file("messages.txt");
        if (!file.is_open()) {
            std::string reply = "MESSAGES:ERROR_OPENING_FILE\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
        } else {
            std::string reply = "MESSAGES:";
            std::string line;
            bool first = true;
            while (std::getline(file, line)) {
                if (!first) reply += "|";  // Разделитель сообщений
                reply += line;
                first = false;
            }
            reply += "\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
        }
    }
    else if (data.rfind("BAN:", 0) == 0) {
        if (!isAdmin) {
            std::string reply = "ERROR: Not authorized\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            return;
        }

        std::string userToBan = data.substr(4);
        bool banned = false;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(userToBan);
            if (it != clients.end()) {
                close(it->second.sockfd);
                clients.erase(it);
                banned = true;
            }
        }

        // TODO: Можно добавить пометку banned в БД, если нужно

        std::string reply = banned ? "RESULT:BAN_OK\n" : "RESULT:BAN_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
        saveLogToFile("BAN command for user " + userToBan + " : " + (banned ? "SUCCESS" : "FAIL"));
    }
    else if (data.rfind("DISCONNECT:", 0) == 0) {
        std::string userToDisconnect = data.substr(11);
        bool disconnected = false;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(userToDisconnect);
            if (it != clients.end()) {
                close(it->second.sockfd);
                clients.erase(it);
                disconnected = true;
            }
        }

        std::string reply = disconnected ? "RESULT:DISCONNECT_OK\n" : "RESULT:DISCONNECT_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
        saveLogToFile("DISCONNECT command for user " + userToDisconnect + " : " + (disconnected ? "SUCCESS" : "FAIL"));
    }
    else {
        // Неизвестная команда — можно игнорировать или логировать
        saveLogToFile("Unknown command from socket " + std::to_string(clientSock) + ": " + data);
    }
}

// === Client handler ===

void handleClient(int clientSock, sockaddr_in clientAddr) {
    char buffer[4096];
    while (true) {
        ssize_t received = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            // Клиент отключился, очищаем данные
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if (it->second.sockfd == clientSock) {
                    close(clientSock);
                    clients.erase(it);
                    break;
                }
            }
            break;
        }
        buffer[received] = '\0';
        std::string data(buffer);
        // Обработка пакета
        handlePacketTCP(clientSock, data, clientAddr);
    }
    close(clientSock);
}

// === Main ===

int main() {
    if (!connectToDB()) {
        std::cerr << "DB connection failed. Exiting.\n";
        return 1;
    }

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_TCP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(serverSock, SOMAXCONN) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Server started on port " << SERVER_TCP_PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSock < 0) {
            perror("accept");
            continue;
        }

        std::thread clientThread(handleClient, clientSock, clientAddr);
        clientThread.detach();
    }

    mysql_close(conn);
    return 0;
}
