#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QMap>
#include <QStringList>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_registerButton_clicked();
    void on_loginButton_clicked();
    void on_sendButton_clicked();
    void on_userList_itemClicked(QListWidgetItem *item);
    void on_readyRead();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString username;
    QString currentChatUser;
    QMap<QString, QStringList> chatHistory;
};

#endif // MAINWINDOW_H
