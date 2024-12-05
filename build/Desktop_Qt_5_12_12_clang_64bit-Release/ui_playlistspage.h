/********************************************************************************
** Form generated from reading UI file 'playlistspage.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYLISTSPAGE_H
#define UI_PLAYLISTSPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <anchortableview.h>

QT_BEGIN_NAMESPACE

class Ui_PlaylistsPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *refreshButton;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_2;
    AnchorTableView *tableView;

    void setupUi(QWidget *PlaylistsPage)
    {
        if (PlaylistsPage->objectName().isEmpty())
            PlaylistsPage->setObjectName(QString::fromUtf8("PlaylistsPage"));
        PlaylistsPage->resize(1002, 649);
        verticalLayout = new QVBoxLayout(PlaylistsPage);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(PlaylistsPage);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font;
        font.setPointSize(36);
        label->setFont(font);

        horizontalLayout->addWidget(label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        refreshButton = new QToolButton(PlaylistsPage);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(refreshButton->sizePolicy().hasHeightForWidth());
        refreshButton->setSizePolicy(sizePolicy);
        refreshButton->setMinimumSize(QSize(30, 30));

        horizontalLayout_2->addWidget(refreshButton);

        pushButton = new QPushButton(PlaylistsPage);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        pushButton_2 = new QPushButton(PlaylistsPage);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);

        tableView = new AnchorTableView(PlaylistsPage);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        verticalLayout->addWidget(tableView);


        retranslateUi(PlaylistsPage);

        QMetaObject::connectSlotsByName(PlaylistsPage);
    } // setupUi

    void retranslateUi(QWidget *PlaylistsPage)
    {
        PlaylistsPage->setWindowTitle(QApplication::translate("PlaylistsPage", "Form", nullptr));
        label->setText(QApplication::translate("PlaylistsPage", "Playlists", nullptr));
        refreshButton->setText(QApplication::translate("PlaylistsPage", "Refresh", nullptr));
        pushButton->setText(QApplication::translate("PlaylistsPage", "New", nullptr));
        pushButton_2->setText(QApplication::translate("PlaylistsPage", "Delete", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PlaylistsPage: public Ui_PlaylistsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYLISTSPAGE_H
