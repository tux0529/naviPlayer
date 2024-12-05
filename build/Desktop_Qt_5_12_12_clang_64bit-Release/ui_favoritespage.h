/********************************************************************************
** Form generated from reading UI file 'favoritespage.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FAVORITESPAGE_H
#define UI_FAVORITESPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FavoritesPage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QPushButton *tracksButton;
    QPushButton *albumsButton;
    QPushButton *artistsButton;
    QSpacerItem *horizontalSpacer;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QWidget *page_2;

    void setupUi(QWidget *FavoritesPage)
    {
        if (FavoritesPage->objectName().isEmpty())
            FavoritesPage->setObjectName(QString::fromUtf8("FavoritesPage"));
        FavoritesPage->resize(1077, 745);
        verticalLayout = new QVBoxLayout(FavoritesPage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(FavoritesPage);
        label->setObjectName(QString::fromUtf8("label"));
        label->setStyleSheet(QString::fromUtf8("font: 36pt \"Arial\";\n"
"font: 36pt \"Hei\";\n"
"font: 36pt \"Heiti SC\";"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tracksButton = new QPushButton(FavoritesPage);
        tracksButton->setObjectName(QString::fromUtf8("tracksButton"));

        horizontalLayout->addWidget(tracksButton);

        albumsButton = new QPushButton(FavoritesPage);
        albumsButton->setObjectName(QString::fromUtf8("albumsButton"));

        horizontalLayout->addWidget(albumsButton);

        artistsButton = new QPushButton(FavoritesPage);
        artistsButton->setObjectName(QString::fromUtf8("artistsButton"));

        horizontalLayout->addWidget(artistsButton);

        horizontalSpacer = new QSpacerItem(668, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        stackedWidget = new QStackedWidget(FavoritesPage);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        stackedWidget->addWidget(page_2);

        verticalLayout->addWidget(stackedWidget);


        retranslateUi(FavoritesPage);

        QMetaObject::connectSlotsByName(FavoritesPage);
    } // setupUi

    void retranslateUi(QWidget *FavoritesPage)
    {
        FavoritesPage->setWindowTitle(QApplication::translate("FavoritesPage", "Form", nullptr));
        label->setText(QApplication::translate("FavoritesPage", "Favorites", nullptr));
        tracksButton->setText(QApplication::translate("FavoritesPage", "Tracks", nullptr));
        albumsButton->setText(QApplication::translate("FavoritesPage", "Albums", nullptr));
        artistsButton->setText(QApplication::translate("FavoritesPage", "Artists", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FavoritesPage: public Ui_FavoritesPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FAVORITESPAGE_H
