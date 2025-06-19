#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000

void receiveMessages(int sockfd) {
    char buffer[1024];
    while (true) {
        ssize_t len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) {
            std::cout << "Disconnected from server.\n";
            break;
        }
        buffer[len] = '\0';
        std::cout << "Server: " << buffer;
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        return 1;
    }

    if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "Connected to server\n";

    // Запускаем поток для получения сообщений от сервера
    std::thread receiver(receiveMessages, sockfd);
    receiver.detach();

    // Ввод и отправка команд пользователем
    std::string line;
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) continue;

        line += "\n"; // Добавляем символ конца строки, чтобы сервер мог парсить
        ssize_t sent = send(sockfd, line.c_str(), line.size(), 0);
        if (sent <= 0) {
            std::cout << "Failed to send message. Exiting...\n";
            break;
        }
        if (line == "exit\n" || line == "quit\n") {
            std::cout << "Exiting client...\n";
            break;
        }
    }

    close(sockfd);
    return 0;
}
