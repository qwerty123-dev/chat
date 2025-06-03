#include <iostream>
#include <string>
#include <thread>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <shared_mutex>
#include <fstream>
#include <stdexcept>
#include <mutex>

#define SERVER_IP "192.168.100.114"
#define SERVER_UDP_PORT 9000

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

    std::string readLine() {
        std::shared_lock lock(mutex);
        file.clear();
        file.seekg(0, std::ios::beg);
        std::string line;
        if (std::getline(file, line)) return line;
        return "";
    }
};

Logger logger;

int udpSock;
sockaddr_in serverAddr;

void listenServer() {
    char buffer[1024];
    sockaddr_in from{};
    socklen_t fromLen = sizeof(from);

    while (true) {
        ssize_t len = recvfrom(udpSock, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&from, &fromLen);
        if (len > 0) {
            buffer[len] = '\0';
            std::string msg(buffer);
            std::cout << "\n[Server]: " << msg << "\n> ";
            std::cout.flush();

            logger.write("[Received] " + msg);
        }
    }
}

int main() {
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) {
        perror("socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_UDP_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid server IP address\n";
        return 1;
    }

    std::thread listener(listenServer);
    listener.detach();

    std::cout << "Commands:\n"
              << "register <username> <password> - Register new user\n"
              << "login <username> <password>    - Login\n"
              << "send <user> <message>          - Send message to user\n"
              << "exit                          - Quit\n";

    while (true) {
        std::string line;
        std::cout << "> ";
        std::getline(std::cin, line);

        std::string msg;

        if (line.rfind("register ", 0) == 0) {
            size_t pos1 = line.find(' ', 9);
            if (pos1 == std::string::npos) {
                std::cout << "Usage: register <username> <password>\n";
                continue;
            }
            std::string username = line.substr(9, pos1 - 9);
            std::string password = line.substr(pos1 + 1);
            msg = "REGISTER:" + username + ":" + password;
        }
        else if (line.rfind("login ", 0) == 0) {
            size_t pos1 = line.find(' ', 6);
            if (pos1 == std::string::npos) {
                std::cout << "Usage: login <username> <password>\n";
                continue;
            }
            std::string username = line.substr(6, pos1 - 6);
            std::string password = line.substr(pos1 + 1);
            msg = "LOGIN:" + username + ":" + password;
        }
        else if (line.rfind("send ", 0) == 0) {
            size_t pos1 = line.find(' ', 5);
            if (pos1 == std::string::npos) {
                std::cout << "Usage: send <user> <message>\n";
                continue;
            }
            std::string receiver = line.substr(5, pos1 - 5);
            std::string message = line.substr(pos1 + 1);
            msg = "MSG:" + receiver + ":" + message;
        }
        else if (line == "exit") {
            break;
        }
        else {
            std::cout << "Unknown command\n";
            continue;
        }

        ssize_t sent = sendto(udpSock, msg.c_str(), msg.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sent == (ssize_t)msg.size()) {
            logger.write("[Sent] " + msg);
        } else {
            std::cerr << "Failed to send full message\n";
        }
    }

    close(udpSock);
    return 0;
}
