#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QStringList>

// Пример класса для локального сохранения логов/пользователей (можно расширять)

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(QObject* parent = nullptr);

    QStringList loadUsers();
    QStringList loadLogs();

    void saveLog(const QString& logLine);

private:
    QString usersFilePath() const;
    QString logsFilePath() const;
};

#endif // DATABASE_H
