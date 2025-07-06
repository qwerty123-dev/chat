/********************************************************************************
** Form generated from reading UI file 'startscreen.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTSCREEN_H
#define UI_STARTSCREEN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StartScreen
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *LoginLineEdit;
    QLineEdit *PasswordLineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QWidget *StartScreen)
    {
        if (StartScreen->objectName().isEmpty())
            StartScreen->setObjectName("StartScreen");
        StartScreen->resize(194, 134);
        verticalLayout = new QVBoxLayout(StartScreen);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        label = new QLabel(StartScreen);
        label->setObjectName("label");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        label_2 = new QLabel(StartScreen);
        label_2->setObjectName("label_2");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, label_2);

        LoginLineEdit = new QLineEdit(StartScreen);
        LoginLineEdit->setObjectName("LoginLineEdit");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, LoginLineEdit);

        PasswordLineEdit = new QLineEdit(StartScreen);
        PasswordLineEdit->setObjectName("PasswordLineEdit");
        PasswordLineEdit->setEchoMode(QLineEdit::EchoMode::Password);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, PasswordLineEdit);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(StartScreen);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(StartScreen);

        QMetaObject::connectSlotsByName(StartScreen);
    } // setupUi

    void retranslateUi(QWidget *StartScreen)
    {
        StartScreen->setWindowTitle(QCoreApplication::translate("StartScreen", "Form", nullptr));
        label->setText(QCoreApplication::translate("StartScreen", "Login:", nullptr));
        label_2->setText(QCoreApplication::translate("StartScreen", "Password:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StartScreen: public Ui_StartScreen {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTSCREEN_H
