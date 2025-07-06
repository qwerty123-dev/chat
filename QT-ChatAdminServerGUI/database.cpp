#include "database.h"
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

Database::Database(QObject* parent) : QObject(parent) {
    // Можно создавать папки/файлы при необходимости
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString Database::usersFilePath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/users.txt";
}

QString Database::logsFilePath() const {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs.txt";
}

QStringList Database::loadUsers() {
    QStringList users;
    QFile file(usersFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return users;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty())
            users.append(line);
    }
    file.close();
    return users;
}

QStringList Database::loadLogs() {
    QStringList logs;
    QFile file(logsFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return logs;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.isEmpty())
            logs.append(line);
    }
    file.close();
    return logs;
}

void Database::saveLog(const QString& logLine) {
    QFile file(logsFilePath());
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << logLine << "\n";
        file.close();
    }
}
