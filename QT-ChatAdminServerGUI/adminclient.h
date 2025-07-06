#ifndef ADMINCLIENT_H
#define ADMINCLIENT_H


#include <QObject>
#include <QTcpSocket>
#include <QStringList>


class AdminClient : public QObject
{
    Q_OBJECT
public:
    explicit AdminClient(QObject* parent = nullptr);
    ~AdminClient();

    void connectToServer(const QString& host, quint16 port);
    void disconnectFromServer();

    void requestUsersList();
    void requestLogs();

    void banUser(const QString& username);
    void disconnectUser(const QString& username);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);

    void usersListReceived(const QStringList& users);
    void logsReceived(const QStringList& logs);
    void actionResult(const QString& result); // for ban/disconnect results

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket* m_socket;
    QString m_buffer;

    void parseMessage(const QString& message);
};

#endif // ADMINCLIENT_H
