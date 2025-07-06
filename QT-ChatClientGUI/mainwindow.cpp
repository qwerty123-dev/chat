#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QHostAddress>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::on_readyRead);

    ui->chatGroupBox->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->messageEdit->setEnabled(false);
}

MainWindow::~MainWindow()
{
    socket->disconnectFromHost();
    socket->close();
    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    socket->connectToHost("127.0.0.1", 1234);
    if (!socket->waitForConnected(1000)) {
        QMessageBox::critical(this, "Connection", "Unable to connect to server.");
    } else {
        QMessageBox::information(this, "Connected", "Connected to server.");
    }
}

void MainWindow::on_registerButton_clicked()
{
    QString user = ui->usernameEdit->text().trimmed();
    QString pass = ui->passwordEdit->text().trimmed();
    if (user.isEmpty() || pass.isEmpty()) return;

    socket->write(QString("REGISTER:%1:%2\n").arg(user, pass).toUtf8());
}

void MainWindow::on_loginButton_clicked()
{
    QString user = ui->usernameEdit->text().trimmed();
    QString pass = ui->passwordEdit->text().trimmed();
    if (user.isEmpty() || pass.isEmpty()) return;

    username = user;
    socket->write(QString("LOGIN:%1:%2\n").arg(user, pass).toUtf8());
}

void MainWindow::on_readyRead()
{
    while (socket->canReadLine()) {
        QString line = socket->readLine().trimmed();

        if (line == "REGISTER_OK") {
            QMessageBox::information(this, "Registration", "Registration successful!");
        } else if (line == "REGISTER_FAIL") {
            QMessageBox::warning(this, "Registration", "Registration failed.");
        } else if (line == "LOGIN_OK") {
            QMessageBox::information(this, "Login", "Login successful!");

            socket->write("GET_USERS\n");
            ui->chatGroupBox->setEnabled(true);
            ui->sendButton->setEnabled(true);
            ui->messageEdit->setEnabled(true);
        } else if (line == "LOGIN_FAIL") {
            QMessageBox::warning(this, "Login", "Login failed.");
        } else if (line.startsWith("USERS:")) {
            QStringList users = line.mid(6).split(",", Qt::SkipEmptyParts);
            ui->userList->clear();
            for (const QString& u : users) {
                if (u != username)
                    ui->userList->addItem(u);
            }
        } else if (line.startsWith("FROM:")) {
            QStringList parts = line.split(":", Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString from = parts[1];
                QString text = parts[2];
                QString full = from + ": " + text;
                chatHistory[from].append(full);

                if (from == currentChatUser) {
                    ui->chatDisplay->append(full);
                } else {
                    QList<QListWidgetItem*> items = ui->userList->findItems(from, Qt::MatchExactly);
                    if (!items.isEmpty())
                        items.first()->setBackground(Qt::yellow);
                }
            }
        }
    }
}

void MainWindow::on_userList_itemClicked(QListWidgetItem *item)
{
    currentChatUser = item->text();
    item->setBackground(Qt::white);
    ui->chatDisplay->clear();
    for (const QString& msg : chatHistory[currentChatUser]) {
        ui->chatDisplay->append(msg);
    }
}

void MainWindow::on_sendButton_clicked()
{
    if (currentChatUser.isEmpty()) return;

    QString msg = ui->messageEdit->text().trimmed();
    if (msg.isEmpty()) return;

    socket->write(QString("MSG:%1:%2\n").arg(currentChatUser, msg).toUtf8());

    QString line = "You: " + msg;
    chatHistory[currentChatUser].append(line);
    ui->chatDisplay->append(line);
    ui->messageEdit->clear();
}


