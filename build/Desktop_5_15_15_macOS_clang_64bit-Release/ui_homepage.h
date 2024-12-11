/********************************************************************************
** Form generated from reading UI file 'homepage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOMEPAGE_H
#define UI_HOMEPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "CardWidget/cardlistwidget.h"

QT_BEGIN_NAMESPACE

class Ui_HomePage
{
public:
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidget;
    QVBoxLayout *verticalLayout_2;
    QLabel *homePageTitleLabel;
    CardListWidget *recentWidget;
    CardListWidget *newestWidget;
    CardListWidget *frequentWidget;
    CardListWidget *randomWidget;

    void setupUi(QWidget *HomePage)
    {
        if (HomePage->objectName().isEmpty())
            HomePage->setObjectName(QString::fromUtf8("HomePage"));
        HomePage->resize(1009, 629);
        HomePage->setAutoFillBackground(true);
        verticalLayout = new QVBoxLayout(HomePage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, 0, -1);
        scrollArea = new QScrollArea(HomePage);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidget = new QWidget();
        scrollAreaWidget->setObjectName(QString::fromUtf8("scrollAreaWidget"));
        scrollAreaWidget->setGeometry(QRect(0, 0, 982, 1194));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollAreaWidget->sizePolicy().hasHeightForWidth());
        scrollAreaWidget->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(scrollAreaWidget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 24, 0);
        homePageTitleLabel = new QLabel(scrollAreaWidget);
        homePageTitleLabel->setObjectName(QString::fromUtf8("homePageTitleLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(homePageTitleLabel->sizePolicy().hasHeightForWidth());
        homePageTitleLabel->setSizePolicy(sizePolicy1);
        homePageTitleLabel->setMinimumSize(QSize(0, 50));
        homePageTitleLabel->setStyleSheet(QString::fromUtf8("font: 36pt \"Arial\";\n"
"font: 36pt \"Hei\";\n"
"font: 36pt \"Heiti SC\";"));

        verticalLayout_2->addWidget(homePageTitleLabel);

        recentWidget = new CardListWidget(scrollAreaWidget);
        recentWidget->setObjectName(QString::fromUtf8("recentWidget"));
        sizePolicy.setHeightForWidth(recentWidget->sizePolicy().hasHeightForWidth());
        recentWidget->setSizePolicy(sizePolicy);
        recentWidget->setMinimumSize(QSize(0, 286));

        verticalLayout_2->addWidget(recentWidget);

        newestWidget = new CardListWidget(scrollAreaWidget);
        newestWidget->setObjectName(QString::fromUtf8("newestWidget"));
        sizePolicy.setHeightForWidth(newestWidget->sizePolicy().hasHeightForWidth());
        newestWidget->setSizePolicy(sizePolicy);
        newestWidget->setMinimumSize(QSize(0, 286));

        verticalLayout_2->addWidget(newestWidget);

        frequentWidget = new CardListWidget(scrollAreaWidget);
        frequentWidget->setObjectName(QString::fromUtf8("frequentWidget"));
        sizePolicy.setHeightForWidth(frequentWidget->sizePolicy().hasHeightForWidth());
        frequentWidget->setSizePolicy(sizePolicy);
        frequentWidget->setMinimumSize(QSize(0, 286));

        verticalLayout_2->addWidget(frequentWidget);

        randomWidget = new CardListWidget(scrollAreaWidget);
        randomWidget->setObjectName(QString::fromUtf8("randomWidget"));
        sizePolicy.setHeightForWidth(randomWidget->sizePolicy().hasHeightForWidth());
        randomWidget->setSizePolicy(sizePolicy);
        randomWidget->setMinimumSize(QSize(0, 286));

        verticalLayout_2->addWidget(randomWidget);

        scrollArea->setWidget(scrollAreaWidget);

        verticalLayout->addWidget(scrollArea);


        retranslateUi(HomePage);

        QMetaObject::connectSlotsByName(HomePage);
    } // setupUi

    void retranslateUi(QWidget *HomePage)
    {
        HomePage->setWindowTitle(QCoreApplication::translate("HomePage", "Form", nullptr));
        homePageTitleLabel->setText(QCoreApplication::translate("HomePage", "Home", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HomePage: public Ui_HomePage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOMEPAGE_H
