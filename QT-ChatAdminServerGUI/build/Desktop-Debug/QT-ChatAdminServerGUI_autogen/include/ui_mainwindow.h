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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QListWidget *UsersListWidget;
    QTextBrowser *MessagesTextBrowser;
    QTextBrowser *LogTextBrowser;
    QHBoxLayout *horizontalLayout;
    QPushButton *BanPushButton;
    QPushButton *DisconnectPushButton;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(484, 311);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        UsersListWidget = new QListWidget(centralwidget);
        UsersListWidget->setObjectName("UsersListWidget");

        horizontalLayout_2->addWidget(UsersListWidget);

        MessagesTextBrowser = new QTextBrowser(centralwidget);
        MessagesTextBrowser->setObjectName("MessagesTextBrowser");

        horizontalLayout_2->addWidget(MessagesTextBrowser);

        LogTextBrowser = new QTextBrowser(centralwidget);
        LogTextBrowser->setObjectName("LogTextBrowser");

        horizontalLayout_2->addWidget(LogTextBrowser);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        BanPushButton = new QPushButton(centralwidget);
        BanPushButton->setObjectName("BanPushButton");

        horizontalLayout->addWidget(BanPushButton);

        DisconnectPushButton = new QPushButton(centralwidget);
        DisconnectPushButton->setObjectName("DisconnectPushButton");

        horizontalLayout->addWidget(DisconnectPushButton);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        BanPushButton->setText(QCoreApplication::translate("MainWindow", "Ban", nullptr));
        DisconnectPushButton->setText(QCoreApplication::translate("MainWindow", "Disconnect", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
