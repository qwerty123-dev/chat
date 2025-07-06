#include "adminclient.h"
#include <QDebug>

AdminClient::AdminClient(QObject* parent) : QObject(parent), m_socket(new QTcpSocket(this)) {
    connect(m_socket, &QTcpSocket::readyRead, this, &AdminClient::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &AdminClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &AdminClient::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &AdminClient::onErrorOccurred);
}

AdminClient::~AdminClient() {
    disconnectFromServer();
}

void AdminClient::connectToServer(const QString& host, quint16 port) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
    m_socket->connectToHost(host, port);
}

void AdminClient::disconnectFromServer() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void AdminClient::requestUsersList() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write("GET_USERS\n");
}

void AdminClient::requestLogs() {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write("GET_LOGS\n");
}

void AdminClient::banUser(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write(("BAN:" + username + "\n").toUtf8());
}

void AdminClient::disconnectUser(const QString& username) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) return;
    m_socket->write(("DISCONNECT:" + username + "\n").toUtf8());
}

void AdminClient::onReadyRead() {
    m_buffer += m_socket->readAll();
    int pos;
    while ((pos = m_buffer.indexOf('\n')) != -1) {
        QString line = m_buffer.left(pos).trimmed();
        m_buffer.remove(0, pos + 1);
        if (!line.isEmpty()) {
            parseMessage(line);
        }
    }
}

void AdminClient::parseMessage(const QString& message) {
    // Пример парсинга ответов от сервера
    // Форматы:
    // USERS: user1,user2,user3
    // LOGS: log line 1 | log line 2 | log line 3
    // RESULT: BAN_OK, BAN_FAIL, DISCONNECT_OK, DISCONNECT_FAIL

    if (message.startsWith("USERS:")) {
        QString usersStr = message.mid(6).trimmed();
        QStringList users = usersStr.split(',', Qt::SkipEmptyParts);
        emit usersListReceived(users);
    }
    else if (message.startsWith("LOGS:")) {
        QString logsStr = message.mid(5).trimmed();
        QStringList logs = logsStr.split('|', Qt::SkipEmptyParts);
        emit logsReceived(logs);
    }
    else if (message.startsWith("RESULT:")) {
        QString res = message.mid(7).trimmed();
        emit actionResult(res);
    }
    else {
        qDebug() << "Unknown message from server:" << message;
    }
}

void AdminClient::onConnected() {
    emit connected();
}

void AdminClient::onDisconnected() {
    emit disconnected();
}

void AdminClient::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}
