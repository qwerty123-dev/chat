/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *connectionLayout;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *connectButton;
    QPushButton *registerButton;
    QPushButton *loginButton;
    QGroupBox *chatGroupBox;
    QHBoxLayout *chatLayout;
    QListWidget *userList;
    QVBoxLayout *messageLayout;
    QTextEdit *chatDisplay;
    QHBoxLayout *sendLayout;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(508, 328);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        connectionLayout = new QHBoxLayout();
        connectionLayout->setObjectName("connectionLayout");
        usernameEdit = new QLineEdit(centralwidget);
        usernameEdit->setObjectName("usernameEdit");

        connectionLayout->addWidget(usernameEdit);

        passwordEdit = new QLineEdit(centralwidget);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        connectionLayout->addWidget(passwordEdit);

        connectButton = new QPushButton(centralwidget);
        connectButton->setObjectName("connectButton");

        connectionLayout->addWidget(connectButton);

        registerButton = new QPushButton(centralwidget);
        registerButton->setObjectName("registerButton");

        connectionLayout->addWidget(registerButton);

        loginButton = new QPushButton(centralwidget);
        loginButton->setObjectName("loginButton");

        connectionLayout->addWidget(loginButton);


        verticalLayout->addLayout(connectionLayout);

        chatGroupBox = new QGroupBox(centralwidget);
        chatGroupBox->setObjectName("chatGroupBox");
        chatLayout = new QHBoxLayout(chatGroupBox);
        chatLayout->setObjectName("chatLayout");
        userList = new QListWidget(chatGroupBox);
        userList->setObjectName("userList");

        chatLayout->addWidget(userList);

        messageLayout = new QVBoxLayout();
        messageLayout->setObjectName("messageLayout");
        chatDisplay = new QTextEdit(chatGroupBox);
        chatDisplay->setObjectName("chatDisplay");
        chatDisplay->setReadOnly(true);

        messageLayout->addWidget(chatDisplay);

        sendLayout = new QHBoxLayout();
        sendLayout->setObjectName("sendLayout");
        messageEdit = new QLineEdit(chatGroupBox);
        messageEdit->setObjectName("messageEdit");

        sendLayout->addWidget(messageEdit);

        sendButton = new QPushButton(chatGroupBox);
        sendButton->setObjectName("sendButton");

        sendLayout->addWidget(sendButton);


        messageLayout->addLayout(sendLayout);


        chatLayout->addLayout(messageLayout);


        verticalLayout->addWidget(chatGroupBox);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Chat Client", nullptr));
        usernameEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Username", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Password", nullptr));
        connectButton->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        registerButton->setText(QCoreApplication::translate("MainWindow", "Register", nullptr));
        loginButton->setText(QCoreApplication::translate("MainWindow", "Login", nullptr));
        chatGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Chat", nullptr));
        messageEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Type a message...", nullptr));
        sendButton->setText(QCoreApplication::translate("MainWindow", "Send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
