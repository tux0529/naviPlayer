/********************************************************************************
** Form generated from reading UI file 'albuminfopage.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALBUMINFOPAGE_H
#define UI_ALBUMINFOPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>
#include <tracktableview.h>

QT_BEGIN_NAMESPACE

class Ui_albumInfoPage
{
public:
    QGridLayout *gridLayout;
    QLabel *posterLabel;
    QLabel *pageTitleLabel;
    QLabel *pageInfoLabel;
    QLabel *pageInfo2Label;
    QHBoxLayout *horizontalLayout;
    QPushButton *playButton;
    QPushButton *insertToQueueButton;
    QPushButton *appendToQueueEndButton;
    QPushButton *favorButton;
    QSpacerItem *horizontalSpacer;
    TrackTableView *tableView;

    void setupUi(QWidget *albumInfoPage)
    {
        if (albumInfoPage->objectName().isEmpty())
            albumInfoPage->setObjectName(QString::fromUtf8("albumInfoPage"));
        albumInfoPage->resize(935, 578);
        gridLayout = new QGridLayout(albumInfoPage);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(12);
        posterLabel = new QLabel(albumInfoPage);
        posterLabel->setObjectName(QString::fromUtf8("posterLabel"));
        posterLabel->setMinimumSize(QSize(180, 180));
        posterLabel->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(posterLabel, 0, 0, 4, 1);

        pageTitleLabel = new QLabel(albumInfoPage);
        pageTitleLabel->setObjectName(QString::fromUtf8("pageTitleLabel"));
        QFont font;
        font.setPointSize(36);
        pageTitleLabel->setFont(font);

        gridLayout->addWidget(pageTitleLabel, 0, 1, 1, 1);

        pageInfoLabel = new QLabel(albumInfoPage);
        pageInfoLabel->setObjectName(QString::fromUtf8("pageInfoLabel"));

        gridLayout->addWidget(pageInfoLabel, 1, 1, 1, 1);

        pageInfo2Label = new QLabel(albumInfoPage);
        pageInfo2Label->setObjectName(QString::fromUtf8("pageInfo2Label"));

        gridLayout->addWidget(pageInfo2Label, 2, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        playButton = new QPushButton(albumInfoPage);
        playButton->setObjectName(QString::fromUtf8("playButton"));

        horizontalLayout->addWidget(playButton);

        insertToQueueButton = new QPushButton(albumInfoPage);
        insertToQueueButton->setObjectName(QString::fromUtf8("insertToQueueButton"));

        horizontalLayout->addWidget(insertToQueueButton);

        appendToQueueEndButton = new QPushButton(albumInfoPage);
        appendToQueueEndButton->setObjectName(QString::fromUtf8("appendToQueueEndButton"));

        horizontalLayout->addWidget(appendToQueueEndButton);

        favorButton = new QPushButton(albumInfoPage);
        favorButton->setObjectName(QString::fromUtf8("favorButton"));

        horizontalLayout->addWidget(favorButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);

        tableView = new TrackTableView(albumInfoPage);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        gridLayout->addWidget(tableView, 4, 0, 1, 2);

        gridLayout->setRowStretch(0, 1);
        gridLayout->setRowStretch(4, 10);

        retranslateUi(albumInfoPage);

        QMetaObject::connectSlotsByName(albumInfoPage);
    } // setupUi

    void retranslateUi(QWidget *albumInfoPage)
    {
        albumInfoPage->setWindowTitle(QCoreApplication::translate("albumInfoPage", "Form", nullptr));
        posterLabel->setText(QCoreApplication::translate("albumInfoPage", "Poster", nullptr));
        pageTitleLabel->setText(QCoreApplication::translate("albumInfoPage", "Brian.Crain", nullptr));
        pageInfoLabel->setText(QCoreApplication::translate("albumInfoPage", "Playlist \302\267 17 songs, 11 hr 9 min 43 sec \302\267 Private", nullptr));
        pageInfo2Label->setText(QCoreApplication::translate("albumInfoPage", "TextLabel", nullptr));
        playButton->setText(QCoreApplication::translate("albumInfoPage", "Play", nullptr));
        insertToQueueButton->setText(QCoreApplication::translate("albumInfoPage", "Insert to queue", nullptr));
#if QT_CONFIG(tooltip)
        appendToQueueEndButton->setToolTip(QCoreApplication::translate("albumInfoPage", "Add to the End of Queue", nullptr));
#endif // QT_CONFIG(tooltip)
        appendToQueueEndButton->setText(QCoreApplication::translate("albumInfoPage", "Append to queue", nullptr));
        favorButton->setText(QCoreApplication::translate("albumInfoPage", "Favor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class albumInfoPage: public Ui_albumInfoPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALBUMINFOPAGE_H
