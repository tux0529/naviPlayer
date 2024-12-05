/********************************************************************************
** Form generated from reading UI file 'cardwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.16
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CARDWIDGET_H
#define UI_CARDWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CardWidget
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *icoLabel;
    QLabel *titleLabel;
    QLabel *artistLabel;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *CardWidget)
    {
        if (CardWidget->objectName().isEmpty())
            CardWidget->setObjectName(QString::fromUtf8("CardWidget"));
        CardWidget->resize(180, 304);
        verticalLayout = new QVBoxLayout(CardWidget);
        verticalLayout->setSpacing(5);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        icoLabel = new QLabel(CardWidget);
        icoLabel->setObjectName(QString::fromUtf8("icoLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(icoLabel->sizePolicy().hasHeightForWidth());
        icoLabel->setSizePolicy(sizePolicy);
        icoLabel->setMinimumSize(QSize(180, 180));
        icoLabel->setBaseSize(QSize(300, 300));
        icoLabel->setPixmap(QPixmap(QString::fromUtf8("demo/getCoverArt.jpeg")));

        verticalLayout->addWidget(icoLabel);

        titleLabel = new QLabel(CardWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(titleLabel->sizePolicy().hasHeightForWidth());
        titleLabel->setSizePolicy(sizePolicy1);
        titleLabel->setMaximumSize(QSize(180, 16777215));
        titleLabel->setBaseSize(QSize(0, 16));
        titleLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(titleLabel);

        artistLabel = new QLabel(CardWidget);
        artistLabel->setObjectName(QString::fromUtf8("artistLabel"));
        sizePolicy1.setHeightForWidth(artistLabel->sizePolicy().hasHeightForWidth());
        artistLabel->setSizePolicy(sizePolicy1);
        artistLabel->setMaximumSize(QSize(180, 16777215));
        artistLabel->setBaseSize(QSize(0, 16));
        artistLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(artistLabel);

        verticalSpacer = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        verticalLayout->setStretch(3, 1);

        retranslateUi(CardWidget);

        QMetaObject::connectSlotsByName(CardWidget);
    } // setupUi

    void retranslateUi(QWidget *CardWidget)
    {
        CardWidget->setWindowTitle(QCoreApplication::translate("CardWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("CardWidget", "\344\270\212\346\265\267\345\276\200\344\272\213 \345\205\255\302\267\347\233\270\344\276\235\344\270\272\344\274\264", nullptr));
        artistLabel->setText(QCoreApplication::translate("CardWidget", "\347\216\213\347\273\264\345\200\251", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CardWidget: public Ui_CardWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CARDWIDGET_H
