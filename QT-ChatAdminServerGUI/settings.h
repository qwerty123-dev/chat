#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings {
public:
    static QString serverHost();
    static quint16 serverPort();
    static QString adminUser();
    static QString adminPass();

    static void setServerHost(const QString& host);
    static void setServerPort(quint16 port);
    static void setAdminUser(const QString& user);
    static void setAdminPass(const QString& pass);

private:
    static QString s_serverHost;
    static quint16 s_serverPort;
    static QString s_adminUser;
    static QString s_adminPass;
};


#endif // SETTINGS_H
