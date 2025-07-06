#include "settings.h"

// Инициализация статических полей (значения по умолчанию)
QString Settings::s_serverHost = "192.168.100.200";
quint16 Settings::s_serverPort = 9000;
QString Settings::s_adminUser = "admin";
QString Settings::s_adminPass = "1234";

QString Settings::serverHost() {
    return s_serverHost;
}

quint16 Settings::serverPort() {
    return s_serverPort;
}

QString Settings::adminUser() {
    return s_adminUser;
}

QString Settings::adminPass() {
    return s_adminPass;
}

void Settings::setServerHost(const QString& host) {
    s_serverHost = host;
}

void Settings::setServerPort(quint16 port) {
    s_serverPort = port;
}

void Settings::setAdminUser(const QString& user) {
    s_adminUser = user;
}

void Settings::setAdminPass(const QString& pass) {
    s_adminPass = pass;
}
