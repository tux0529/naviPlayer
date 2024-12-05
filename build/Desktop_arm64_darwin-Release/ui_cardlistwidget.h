/********************************************************************************
** Form generated from reading UI file 'cardlistwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.16
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CARDLISTWIDGET_H
#define UI_CARDLISTWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CardListWidget
{
public:
    QGridLayout *gridLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidget;
    QHBoxLayout *hLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *titleLabel;
    QPushButton *rightPushButton;
    QPushButton *leftPushButton;

    void setupUi(QWidget *CardListWidget)
    {
        if (CardListWidget->objectName().isEmpty())
            CardListWidget->setObjectName(QString::fromUtf8("CardListWidget"));
        CardListWidget->resize(1003, 205);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CardListWidget->sizePolicy().hasHeightForWidth());
        CardListWidget->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(CardListWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, -1, 0, -1);
        scrollArea = new QScrollArea(CardListWidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
        scrollArea->setSizePolicy(sizePolicy1);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidget = new QWidget();
        scrollAreaWidget->setObjectName(QString::fromUtf8("scrollAreaWidget"));
        scrollAreaWidget->setGeometry(QRect(0, 0, 1003, 20));
        sizePolicy.setHeightForWidth(scrollAreaWidget->sizePolicy().hasHeightForWidth());
        scrollAreaWidget->setSizePolicy(sizePolicy);
        hLayout = new QHBoxLayout(scrollAreaWidget);
        hLayout->setObjectName(QString::fromUtf8("hLayout"));
        hLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(950, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayout->addItem(horizontalSpacer);

        scrollArea->setWidget(scrollAreaWidget);

        gridLayout->addWidget(scrollArea, 1, 0, 1, 3);

        titleLabel = new QLabel(CardListWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font: 16pt \"Heiti SC\";"));

        gridLayout->addWidget(titleLabel, 0, 0, 1, 1);

        rightPushButton = new QPushButton(CardListWidget);
        rightPushButton->setObjectName(QString::fromUtf8("rightPushButton"));

        gridLayout->addWidget(rightPushButton, 0, 2, 1, 1);

        leftPushButton = new QPushButton(CardListWidget);
        leftPushButton->setObjectName(QString::fromUtf8("leftPushButton"));

        gridLayout->addWidget(leftPushButton, 0, 1, 1, 1);

        gridLayout->setColumnStretch(0, 1);

        retranslateUi(CardListWidget);

        QMetaObject::connectSlotsByName(CardListWidget);
    } // setupUi

    void retranslateUi(QWidget *CardListWidget)
    {
        CardListWidget->setWindowTitle(QCoreApplication::translate("CardListWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("CardListWidget", "Recently Played", nullptr));
        rightPushButton->setText(QCoreApplication::translate("CardListWidget", ">", nullptr));
        leftPushButton->setText(QCoreApplication::translate("CardListWidget", "<", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CardListWidget: public Ui_CardListWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CARDLISTWIDGET_H
