#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"

#include <QTcpSocket>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    socket(new QTcpSocket(this))
{
    ui->setupUi(this);

    // Подключаем сигналы сокета
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &MainWindow::onSocketerrorOccured);

    // Подключаемся к серверу с параметрами из Settings
    socket->connectToHost(Settings::serverHost(), Settings::serverPort());

    // Можно сразу запросить список пользователей и логи (после подключения)
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        requestUsersList();
        requestLogs();
        requestMessages();
    });
}

MainWindow::~MainWindow()
{
    updateTimer->stop();
    socket->disconnectFromHost();
    delete ui;
}

void MainWindow::onSocketConnected()
{
    ui->LogTextBrowser->append("Connected to server at " + Settings::serverHost() + ":" + QString::number(Settings::serverPort()));
    requestUsersList();
    requestLogs();
    requestMessages();
    updateTimer->start(10000);
}

void MainWindow::onSocketReadyRead()
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        parseServerMessage(QString::fromUtf8(line));
    }
}

void MainWindow::onSocketerrorOccured(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QMessageBox::warning(this, "Socket Error", socket->errorString());
}

void MainWindow::requestUsersList()
{
    // Пример запроса списка пользователей (предположим, протокол сервер/клиент поддерживает такую команду)
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write("GET_USERS\n");
    }
}

void MainWindow::requestLogs()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write("GET_LOGS\n");
    }
}

void MainWindow::requestMessages()
{
    if(socket->state() == QAbstractSocket::ConnectedState) {
        socket->write("GET_MESSAGES\n");
    }
}


void MainWindow::parseServerMessage(const QString& msg)
{
    // Пример: сервер шлет строки, начинающиеся с ключевых слов
    if (msg.startsWith("USERS:")) {
        // Формат: USERS:user1,user2,user3
        QString usersStr = msg.mid(6);
        QStringList users = usersStr.split(',', Qt::SkipEmptyParts);
        updateUsersList(users);
    } else if (msg.startsWith("LOGS:")) {
        QString allLogs = msg.mid(4);
        QStringList logLines = allLogs.split('|', Qt::SkipEmptyParts);
        updateLogs(logLines);
    } else if (msg.startsWith("MESSAGES:")) {
        QString allMessages = msg.mid(9);
        QStringList lines = allMessages.split('|');
        updateMessages(lines);

    } else {
        ui->LogTextBrowser->append("Unknown message: " + msg);
    }
}

void MainWindow::updateUsersList(const QStringList &users)
{
    ui->UsersListWidget->clear();
    ui->UsersListWidget->addItems(users);
}

void MainWindow::updateLogs(const QStringList& logLines)
{
    ui->LogTextBrowser->clear();
    for(const QString& line : logLines) {
        ui->LogTextBrowser->append(line);
    }
}


void MainWindow::updateMessages(const QStringList &messages)
{
    ui->MessagesTextBrowser->clear();
    for(const QString& msg : messages) {
        ui->MessagesTextBrowser->append(msg);
    }
}


void MainWindow::on_BanPushButton_clicked()
{
    QString user = ui->UsersListWidget->currentItem() ? ui->UsersListWidget->currentItem()->text() : QString();
    if (user.isEmpty()) {
        QMessageBox::warning(this, "Select user", "Please select a user to ban.");
        return;
    }
    QString cmd = "BAN:" + user + "\n";
    socket->write(cmd.toUtf8());
    ui->LogTextBrowser->append("Sent ban command for user: " + user);
}

void MainWindow::on_DisconnectPushButton_clicked()
{
    QString user = ui->UsersListWidget->currentItem() ? ui->UsersListWidget->currentItem()->text() : QString();
    if (user.isEmpty()) {
        QMessageBox::warning(this, "Select user", "Please select a user to disconnect.");
        return;
    }
    QString cmd = "DISCONNECT:" + user + "\n";
    socket->write(cmd.toUtf8());
    ui->LogTextBrowser->append("Sent disconnect command for user: " + user);
}

