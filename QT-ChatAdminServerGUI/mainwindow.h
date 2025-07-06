#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "adminclient.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_BanPushButton_clicked();
    void on_DisconnectPushButton_clicked();

    void onSocketReadyRead();
    void onSocketConnected();
    void onSocketerrorOccured(QAbstractSocket::SocketError socketError);

    void requestUsersList();
    void requestLogs();
    void requestMessages();

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QTimer* updateTimer;
    QStringList displayedLogs;

    void parseServerMessage(const QString& msg);
    void updateUsersList(const QStringList &users);
    void updateLogs(const QStringList& logLines);
    void updateMessages(const QStringList &messages);
};
#endif // MAINWINDOW_H
