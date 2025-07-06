#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <fstream>

#define SERVER_TCP_PORT 1234

// === Логи ===
std::mutex logFileMutex;
void saveLogToFile(const std::string& logLine) {
    std::lock_guard<std::mutex> lock(logFileMutex);
    std::ofstream file("log.txt", std::ios::app);
    if (file.is_open()) file << logLine << std::endl;
}

// === Сообщения ===
std::mutex messageFileMutex;
void saveMessageToFile(const std::string& sender, const std::string& receiver, const std::string& message) {
    std::lock_guard<std::mutex> lock(messageFileMutex);
    std::ofstream file("messages.txt", std::ios::app);
    if (file.is_open()) file << sender << " -> " << receiver << " : " << message << "\n";
}

// === Клиенты ===
struct ClientInfo {
    int sockfd;
    sockaddr_in addr;
};

std::map<std::string, ClientInfo> clients;
std::mutex clients_mutex;

MYSQL* conn;

// === DB ===
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
    if (!conn) return false;

    if (!mysql_real_connect(conn, "localhost", "root", "root", nullptr, 0, nullptr, 0)) {
        std::cerr << "MySQL connection failed: " << mysql_error(conn) << "\n";
        return false;
    }

    mysql_query(conn, "CREATE DATABASE IF NOT EXISTS chatdb");
    mysql_select_db(conn, "chatdb");

    const char* createUsers = "CREATE TABLE IF NOT EXISTS users ("
                              "username VARCHAR(100) PRIMARY KEY,"
                              "password VARCHAR(100),"
                              "banned BOOLEAN DEFAULT 0)";
    const char* createMessages = "CREATE TABLE IF NOT EXISTS messages ("
                                 "id INT AUTO_INCREMENT PRIMARY KEY,"
                                 "sender VARCHAR(100),"
                                 "receiver VARCHAR(100),"
                                 "text TEXT)";

    mysql_query(conn, createUsers);
    mysql_query(conn, createMessages);

    return true;
}

void saveMessageToDB(const std::string& sender, const std::string& receiver, const std::string& text) {
    std::string query = "INSERT INTO messages(sender, receiver, text) VALUES('" +
                        escapeString(sender) + "', '" + escapeString(receiver) + "', '" + escapeString(text) + "')";
    mysql_query(conn, query.c_str());
}

bool validateLogin(const std::string& user, const std::string& pass) {
    std::string query = "SELECT * FROM users WHERE username='" + escapeString(user) +
                        "' AND password='" + escapeString(pass) + "' AND banned = 0";
    if (mysql_query(conn, query.c_str()) != 0) return false;
    MYSQL_RES* res = mysql_store_result(conn);
    bool valid = mysql_num_rows(res) > 0;
    mysql_free_result(res);
    return valid;
}

bool registerUser(const std::string& user, const std::string& pass) {
    std::string query = "INSERT INTO users(username, password) VALUES('" +
                        escapeString(user) + "', '" + escapeString(pass) + "')";
    return mysql_query(conn, query.c_str()) == 0;
}

// === Обработка команд ===
void handlePacketTCP(int clientSock, const std::string& data, sockaddr_in clientAddr) {
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

        if (validateLogin(user, pass)) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients[user] = ClientInfo{clientSock, clientAddr};
            }
            send(clientSock, "LOGIN_OK\n", 9, 0);
            saveLogToFile("LOGIN " + user + " : SUCCESS");
        } else {
            send(clientSock, "LOGIN_FAIL\n", 11, 0);
            saveLogToFile("LOGIN " + user + " : FAIL");
        }
    }
    else if (data.rfind("MSG:", 0) == 0) {
        size_t pos1 = data.find(':', 4);
        if (pos1 == std::string::npos) return;
        std::string receiver = data.substr(4, pos1 - 4);
        std::string message = data.substr(pos1 + 1);

        std::string sender;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (auto& pair : clients) {
                if (pair.second.sockfd == clientSock) {
                    sender = pair.first;
                    break;
                }
            }
        }
        if (sender.empty()) return;

        saveMessageToDB(sender, receiver, message);
        saveMessageToFile(sender, receiver, message);

        std::lock_guard<std::mutex> lock(clients_mutex);
        if (clients.count(receiver)) {
            std::string out = "FROM:" + sender + ":" + message + "\n";
            send(clients[receiver].sockfd, out.c_str(), out.size(), 0);
        }
    }
    else if (data == "GET_USERS") {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::string usersList = "USERS:";
        for (const auto& pair : clients) usersList += pair.first + ",";
        if (!clients.empty()) usersList.pop_back();
        usersList += "\n";
        send(clientSock, usersList.c_str(), usersList.size(), 0);
    }
    else if (data == "GET_LOGS") {
        std::ifstream logFile("log.txt");
        std::string reply = "LOGS:";
        std::string line;
        bool first = true;
        while (std::getline(logFile, line)) {
            if (!first) reply += "|";
            reply += line;
            first = false;
        }
        reply += "\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
    }
    else if (data == "GET_MESSAGES") {
        std::ifstream file("messages.txt");
        std::string reply = "MESSAGES:";
        std::string line;
        bool first = true;
        while (std::getline(file, line)) {
            if (!first) reply += "|";
            reply += line;
            first = false;
        }
        reply += "\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
    }
    else if (data.rfind("BAN:", 0) == 0) {
        std::string username = data.substr(4);
        bool banned = false;

        std::string q = "UPDATE users SET banned=1 WHERE username='" + escapeString(username) + "'";
        if (mysql_query(conn, q.c_str()) == 0) {
            banned = mysql_affected_rows(conn) > 0;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(username);
            if (it != clients.end()) {
                close(it->second.sockfd);
                clients.erase(it);
            }
        }

        std::string reply = banned ? "RESULT:BAN_OK\n" : "RESULT:BAN_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
        saveLogToFile("BAN " + username + " : " + (banned ? "SUCCESS" : "FAIL"));
    }
    else if (data.rfind("DISCONNECT:", 0) == 0) {
        std::string username = data.substr(11);
        bool disconnected = false;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(username);
            if (it != clients.end()) {
                close(it->second.sockfd);
                clients.erase(it);
                disconnected = true;
            }
        }

        std::string reply = disconnected ? "RESULT:DISCONNECT_OK\n" : "RESULT:DISCONNECT_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
        saveLogToFile("DISCONNECT " + username + " : " + (disconnected ? "SUCCESS" : "FAIL"));
    }
    else {
        saveLogToFile("Unknown command from socket " + std::to_string(clientSock) + ": " + data);
    }
}

// === Клиентский поток ===
void handleClient(int clientSock, sockaddr_in clientAddr) {
    char buffer[1024];
    std::string partialMsg;

    while (true) {
        ssize_t len = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        buffer[len] = '\0';
        partialMsg += buffer;

        size_t pos;
        while ((pos = partialMsg.find('\n')) != std::string::npos) {
            std::string msg = partialMsg.substr(0, pos);
            partialMsg.erase(0, pos + 1);
            handlePacketTCP(clientSock, msg, clientAddr);
        }
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.sockfd == clientSock) {
                saveLogToFile("Client disconnected: " + it->first);
                clients.erase(it);
                break;
            }
        }
    }
    close(clientSock);
}

// === main ===
int main() {
    if (!connectToDB()) return 1;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket"); return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_TCP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind"); close(sockfd); return 1;
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen"); close(sockfd); return 1;
    }

    std::cout << "Server running on port " << SERVER_TCP_PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(sockfd, (sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0) {
            perror("accept"); continue;
        }
        std::thread(handleClient, clientSock, clientAddr).detach();
    }

    close(sockfd);
    mysql_close(conn);
    return 0;
}
