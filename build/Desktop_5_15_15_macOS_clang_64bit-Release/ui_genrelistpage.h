/********************************************************************************
** Form generated from reading UI file 'genrelistpage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GENRELISTPAGE_H
#define UI_GENRELISTPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <anchortableview.h>

QT_BEGIN_NAMESPACE

class Ui_GenreListPage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    AnchorTableView *tableView;

    void setupUi(QWidget *GenreListPage)
    {
        if (GenreListPage->objectName().isEmpty())
            GenreListPage->setObjectName(QString::fromUtf8("GenreListPage"));
        GenreListPage->resize(1059, 631);
        verticalLayout = new QVBoxLayout(GenreListPage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(GenreListPage);
        label->setObjectName(QString::fromUtf8("label"));
        label->setStyleSheet(QString::fromUtf8("font: 36pt \"Arial\";\n"
"font: 36pt \"Hei\";\n"
"font: 36pt \"Heiti SC\";"));

        verticalLayout->addWidget(label);

        tableView = new AnchorTableView(GenreListPage);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        verticalLayout->addWidget(tableView);


        retranslateUi(GenreListPage);

        QMetaObject::connectSlotsByName(GenreListPage);
    } // setupUi

    void retranslateUi(QWidget *GenreListPage)
    {
        GenreListPage->setWindowTitle(QCoreApplication::translate("GenreListPage", "Form", nullptr));
        label->setText(QCoreApplication::translate("GenreListPage", "Genres", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GenreListPage: public Ui_GenreListPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GENRELISTPAGE_H
