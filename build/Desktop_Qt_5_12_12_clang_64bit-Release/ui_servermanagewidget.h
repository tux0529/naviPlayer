/********************************************************************************
** Form generated from reading UI file 'servermanagewidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERMANAGEWIDGET_H
#define UI_SERVERMANAGEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ServerManageWidget
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *shadowWidget;
    QVBoxLayout *verticalLayout_2;
    QWidget *titleWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *titleLabel;
    QPushButton *closeButton;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_6;
    QToolButton *addButton;
    QToolButton *okButton;
    QComboBox *serverComboBox;
    QFrame *frame;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_2;
    QLineEdit *nameEdit;
    QLabel *label_3;
    QLineEdit *urlEdit;
    QLabel *label_4;
    QLineEdit *userEdit;
    QLabel *label_5;
    QLineEdit *passwordEdit;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_2;
    QLabel *resultLabel;
    QPushButton *cancelButton;
    QPushButton *submitButton;
    QLabel *label_7;

    void setupUi(QDialog *ServerManageWidget)
    {
        if (ServerManageWidget->objectName().isEmpty())
            ServerManageWidget->setObjectName(QString::fromUtf8("ServerManageWidget"));
        ServerManageWidget->resize(380, 320);
        verticalLayout = new QVBoxLayout(ServerManageWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        shadowWidget = new QWidget(ServerManageWidget);
        shadowWidget->setObjectName(QString::fromUtf8("shadowWidget"));
        verticalLayout_2 = new QVBoxLayout(shadowWidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        titleWidget = new QWidget(shadowWidget);
        titleWidget->setObjectName(QString::fromUtf8("titleWidget"));
        horizontalLayout = new QHBoxLayout(titleWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 0, 0, 0);
        titleLabel = new QLabel(titleWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(titleLabel);

        closeButton = new QPushButton(titleWidget);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setMinimumSize(QSize(36, 0));

        horizontalLayout->addWidget(closeButton);

        horizontalLayout->setStretch(0, 1);

        verticalLayout_2->addWidget(titleWidget);

        widget_2 = new QWidget(shadowWidget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setSpacing(18);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(24, 24, 24, 24);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_6 = new QLabel(widget_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_3->addWidget(label_6);

        addButton = new QToolButton(widget_2);
        addButton->setObjectName(QString::fromUtf8("addButton"));

        horizontalLayout_3->addWidget(addButton);

        okButton = new QToolButton(widget_2);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout_3->addWidget(okButton);

        horizontalLayout_3->setStretch(0, 1);

        verticalLayout_3->addLayout(horizontalLayout_3);

        serverComboBox = new QComboBox(widget_2);
        serverComboBox->setObjectName(QString::fromUtf8("serverComboBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(serverComboBox->sizePolicy().hasHeightForWidth());
        serverComboBox->setSizePolicy(sizePolicy);
        serverComboBox->setMinimumSize(QSize(0, 32));
        serverComboBox->setEditable(true);

        verticalLayout_3->addWidget(serverComboBox);

        frame = new QFrame(widget_2);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setMinimumSize(QSize(0, 80));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_4 = new QVBoxLayout(frame);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_4->addWidget(label_2);

        nameEdit = new QLineEdit(frame);
        nameEdit->setObjectName(QString::fromUtf8("nameEdit"));
        nameEdit->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(nameEdit->sizePolicy().hasHeightForWidth());
        nameEdit->setSizePolicy(sizePolicy1);
        nameEdit->setMinimumSize(QSize(0, 32));

        verticalLayout_4->addWidget(nameEdit);

        label_3 = new QLabel(frame);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_4->addWidget(label_3);

        urlEdit = new QLineEdit(frame);
        urlEdit->setObjectName(QString::fromUtf8("urlEdit"));
        urlEdit->setEnabled(true);
        sizePolicy1.setHeightForWidth(urlEdit->sizePolicy().hasHeightForWidth());
        urlEdit->setSizePolicy(sizePolicy1);
        urlEdit->setMinimumSize(QSize(0, 32));

        verticalLayout_4->addWidget(urlEdit);

        label_4 = new QLabel(frame);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_4->addWidget(label_4);

        userEdit = new QLineEdit(frame);
        userEdit->setObjectName(QString::fromUtf8("userEdit"));
        userEdit->setEnabled(true);
        sizePolicy1.setHeightForWidth(userEdit->sizePolicy().hasHeightForWidth());
        userEdit->setSizePolicy(sizePolicy1);
        userEdit->setMinimumSize(QSize(0, 32));

        verticalLayout_4->addWidget(userEdit);

        label_5 = new QLabel(frame);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        verticalLayout_4->addWidget(label_5);

        passwordEdit = new QLineEdit(frame);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setEnabled(true);
        sizePolicy1.setHeightForWidth(passwordEdit->sizePolicy().hasHeightForWidth());
        passwordEdit->setSizePolicy(sizePolicy1);
        passwordEdit->setMinimumSize(QSize(0, 32));
        passwordEdit->setEchoMode(QLineEdit::Password);

        verticalLayout_4->addWidget(passwordEdit);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(-1, 0, -1, 0);
        pushButton_2 = new QPushButton(frame);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);

        resultLabel = new QLabel(frame);
        resultLabel->setObjectName(QString::fromUtf8("resultLabel"));

        horizontalLayout_2->addWidget(resultLabel);

        cancelButton = new QPushButton(frame);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout_2->addWidget(cancelButton);

        submitButton = new QPushButton(frame);
        submitButton->setObjectName(QString::fromUtf8("submitButton"));

        horizontalLayout_2->addWidget(submitButton);

        horizontalLayout_2->setStretch(1, 1);

        verticalLayout_4->addLayout(horizontalLayout_2);


        verticalLayout_3->addWidget(frame);

        label_7 = new QLabel(widget_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setOpenExternalLinks(true);

        verticalLayout_3->addWidget(label_7);

        verticalLayout_3->setStretch(3, 1);

        verticalLayout_2->addWidget(widget_2);

        verticalLayout_2->setStretch(1, 1);

        verticalLayout->addWidget(shadowWidget);


        retranslateUi(ServerManageWidget);

        QMetaObject::connectSlotsByName(ServerManageWidget);
    } // setupUi

    void retranslateUi(QDialog *ServerManageWidget)
    {
        ServerManageWidget->setWindowTitle(QApplication::translate("ServerManageWidget", "Form", nullptr));
        titleLabel->setText(QApplication::translate("ServerManageWidget", "Manage Server", nullptr));
        closeButton->setText(QApplication::translate("ServerManageWidget", "X", nullptr));
        label_6->setText(QApplication::translate("ServerManageWidget", "Subsonic/ Navidrome Server", nullptr));
        addButton->setText(QApplication::translate("ServerManageWidget", "Add Server", nullptr));
        okButton->setText(QApplication::translate("ServerManageWidget", "OK", nullptr));
        label_2->setText(QApplication::translate("ServerManageWidget", "Server Name", nullptr));
        label_3->setText(QApplication::translate("ServerManageWidget", "Url", nullptr));
        label_4->setText(QApplication::translate("ServerManageWidget", "User Name", nullptr));
        label_5->setText(QApplication::translate("ServerManageWidget", "Password", nullptr));
        pushButton_2->setText(QApplication::translate("ServerManageWidget", "Test", nullptr));
        resultLabel->setText(QString());
        cancelButton->setText(QApplication::translate("ServerManageWidget", "Cancel", nullptr));
        submitButton->setText(QApplication::translate("ServerManageWidget", "Add", nullptr));
        label_7->setText(QApplication::translate("ServerManageWidget", "<a href='https://www.navidrome.org/docs/installation/'>How to Install your own Navidrome server.</a>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerManageWidget: public Ui_ServerManageWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERMANAGEWIDGET_H
