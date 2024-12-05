/********************************************************************************
** Form generated from reading UI file 'artistspage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.16
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ARTISTSPAGE_H
#define UI_ARTISTSPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <anchortableview.h>

QT_BEGIN_NAMESPACE

class Ui_ArtistsPage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    AnchorTableView *tableView;

    void setupUi(QWidget *ArtistsPage)
    {
        if (ArtistsPage->objectName().isEmpty())
            ArtistsPage->setObjectName(QString::fromUtf8("ArtistsPage"));
        ArtistsPage->resize(730, 540);
        verticalLayout = new QVBoxLayout(ArtistsPage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(ArtistsPage);
        label->setObjectName(QString::fromUtf8("label"));
        label->setStyleSheet(QString::fromUtf8("font: 36pt \"Arial\";\n"
"font: 36pt \"Hei\";\n"
"font: 36pt \"Heiti SC\";"));

        verticalLayout->addWidget(label);

        tableView = new AnchorTableView(ArtistsPage);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        verticalLayout->addWidget(tableView);


        retranslateUi(ArtistsPage);

        QMetaObject::connectSlotsByName(ArtistsPage);
    } // setupUi

    void retranslateUi(QWidget *ArtistsPage)
    {
        ArtistsPage->setWindowTitle(QCoreApplication::translate("ArtistsPage", "Form", nullptr));
        label->setText(QCoreApplication::translate("ArtistsPage", "Artists", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ArtistsPage: public Ui_ArtistsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ARTISTSPAGE_H
