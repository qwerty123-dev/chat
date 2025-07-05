#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <fstream>
#include <stdexcept>

#define SERVER_TCP_PORT 9000

// === Logger ===
class Logger {
private:
    std::fstream file;
    mutable std::shared_mutex mutex;

public:
    Logger(const std::string& filename = "log.txt") {
        file.open(filename, std::ios::in | std::ios::out | std::ios::app);
        if (!file.is_open()) {
            file.clear();
            file.open(filename, std::ios::out);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::app);
            if (!file.is_open()) throw std::runtime_error("Cannot open log file");
        }
    }
    ~Logger() {
        if (file.is_open()) file.close();
    }
    void write(const std::string& logLine) {
        std::unique_lock lock(mutex);
        file.clear();
        file.seekp(0, std::ios::end);
        file << logLine << std::endl;
        file.flush();
    }
};

Logger logger;


// === Messages File ===
std::mutex messageFileMutex;

void saveMessageToFile(const std::string& sender, const std::string& reciever, const std::string& message)
{
    std::lock_guard<std::mutex> lock(messageFileMutex);
    std::ofstream file("messages.txt", std::ios::app);
    if(file.is_open()) {
        file << sender << " -> " << reciever << " : " << message << "\n";
    }
}

// === Client Info ===
struct ClientInfo {
    int sockfd;
    sockaddr_in addr;
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
                              "password VARCHAR(100))";

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
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB insert error: " << mysql_error(conn) << "\n";
    }
}

bool validateLogin(const std::string& user, const std::string& pass) {
    std::string query = "SELECT * FROM users WHERE username='" + escapeString(user) + "' AND password='" + escapeString(pass) + "'";
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB select error: " << mysql_error(conn) << "\n";
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    bool valid = mysql_num_rows(res) > 0;
    mysql_free_result(res);
    return valid;
}

bool registerUser(const std::string& user, const std::string& pass) {
    std::string query = "INSERT INTO users(username, password) VALUES('" + escapeString(user) + "', '" + escapeString(pass) + "')";
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "DB insert error: " << mysql_error(conn) << "\n";
        return false;
    }
    return true;
}

// === Message handling ===

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

        logger.write("REGISTER " + user + " : " + (success ? "SUCCESS" : "FAIL"));
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
            std::string reply = "LOGIN_OK\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            logger.write("LOGIN " + user + " : SUCCESS");
        } else {
            std::string reply = "LOGIN_FAIL\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            logger.write("LOGIN " + user + " : FAIL");
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
        //logger.write("MSG from " + sender + " to " + receiver + ": " + message);
        saveMessageToFile(sender, reciever, message);

        std::lock_guard<std::mutex> lock(clients_mutex);
        if (clients.count(receiver)) {
            std::string out = "FROM:" + sender + ":" + message + "\n";
            send(clients[receiver].sockfd, out.c_str(), out.size(), 0);
            //logger.write("Sent to " + receiver + ": " + message);
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
    } else if (data == "GET_MESSAGES") {
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
        std::string username = data.substr(4);
        bool banned = false;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto it = clients.find(username);
            if (it != clients.end()) {
                close(it->second.sockfd);
                clients.erase(it);
                banned = true;
            }
        }

        // TODO: Можно добавить пометку banned в БД
        // std::string q = "UPDATE users SET banned=1 WHERE username='" + escapeString(username) + "'";
        // mysql_query(conn, q.c_str());

        std::string reply = banned ? "RESULT:BAN_OK\n" : "RESULT:BAN_FAIL\n";
        send(clientSock, reply.c_str(), reply.size(), 0);
        logger.write("BAN command for user " + username + " : " + (banned ? "SUCCESS" : "FAIL"));
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
        logger.write("DISCONNECT command for user " + username + " : " + (disconnected ? "SUCCESS" : "FAIL"));
    } 
    else {
        // Неизвестная команда — можно игнорировать или логировать
        logger.write("Unknown command from socket " + std::to_string(clientSock) + ": " + data);
    }
}


// === Client handler ===

void handleClient(int clientSock, sockaddr_in clientAddr) {
    char buffer[1024];
    std::string partialMsg;

    while (true) {
        ssize_t len = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;  // клиент отключился или ошибка
        buffer[len] = '\0';
        partialMsg += buffer;

        // Обработка сообщений, разделенных '\n'
        size_t pos;
        while ((pos = partialMsg.find('\n')) != std::string::npos) {
            std::string msg = partialMsg.substr(0, pos);
            partialMsg.erase(0, pos + 1);
            handlePacketTCP(clientSock, msg, clientAddr);
        }
    }

    // Удаляем клиента при отключении
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.sockfd == clientSock) {
                logger.write("Client disconnected: " + it->first);
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
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_TCP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        perror("listen");
        close(sockfd);
        return 1;
    }

    std::cout << "TCP Server started on port " << SERVER_TCP_PORT << "\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(sockfd, (sockaddr*)&clientAddr, &clientLen);
        if (clientSock < 0) {
            perror("accept");
            continue;
        }

        std::thread(handleClient, clientSock, clientAddr).detach();
    }

    close(sockfd);
    mysql_close(conn);
    return 0;
}
