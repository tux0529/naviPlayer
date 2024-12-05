/********************************************************************************
** Form generated from reading UI file 'TrackListPage.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TrackListPage_H
#define UI_TrackListPage_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>
#include <anchortableview.h>

QT_BEGIN_NAMESPACE

class Ui_TrackListPage
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
    AnchorTableView *tableView;

    void setupUi(QWidget *TrackListPage)
    {
        if (TrackListPage->objectName().isEmpty())
            TrackListPage->setObjectName(QString::fromUtf8("TrackListPage"));
        TrackListPage->resize(935, 578);
        gridLayout = new QGridLayout(TrackListPage);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(12);
        posterLabel = new QLabel(TrackListPage);
        posterLabel->setObjectName(QString::fromUtf8("posterLabel"));
        posterLabel->setMinimumSize(QSize(180, 180));
        posterLabel->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(posterLabel, 0, 0, 4, 1);

        pageTitleLabel = new QLabel(TrackListPage);
        pageTitleLabel->setObjectName(QString::fromUtf8("pageTitleLabel"));
        QFont font;
        font.setPointSize(36);
        pageTitleLabel->setFont(font);

        gridLayout->addWidget(pageTitleLabel, 0, 1, 1, 1);

        pageInfoLabel = new QLabel(TrackListPage);
        pageInfoLabel->setObjectName(QString::fromUtf8("pageInfoLabel"));

        gridLayout->addWidget(pageInfoLabel, 1, 1, 1, 1);

        pageInfo2Label = new QLabel(TrackListPage);
        pageInfo2Label->setObjectName(QString::fromUtf8("pageInfo2Label"));

        gridLayout->addWidget(pageInfo2Label, 2, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        playButton = new QPushButton(TrackListPage);
        playButton->setObjectName(QString::fromUtf8("playButton"));

        horizontalLayout->addWidget(playButton);

        insertToQueueButton = new QPushButton(TrackListPage);
        insertToQueueButton->setObjectName(QString::fromUtf8("insertToQueueButton"));

        horizontalLayout->addWidget(insertToQueueButton);

        appendToQueueEndButton = new QPushButton(TrackListPage);
        appendToQueueEndButton->setObjectName(QString::fromUtf8("appendToQueueEndButton"));

        horizontalLayout->addWidget(appendToQueueEndButton);

        favorButton = new QPushButton(TrackListPage);
        favorButton->setObjectName(QString::fromUtf8("favorButton"));

        horizontalLayout->addWidget(favorButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout, 3, 1, 1, 1);

        tableView = new AnchorTableView(TrackListPage);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        gridLayout->addWidget(tableView, 4, 0, 1, 2);

        gridLayout->setRowStretch(0, 1);
        gridLayout->setRowStretch(4, 10);

        retranslateUi(TrackListPage);

        QMetaObject::connectSlotsByName(TrackListPage);
    } // setupUi

    void retranslateUi(QWidget *TrackListPage)
    {
        TrackListPage->setWindowTitle(QApplication::translate("TrackListPage", "Form", nullptr));
        posterLabel->setText(QApplication::translate("TrackListPage", "Poster", nullptr));
        pageTitleLabel->setText(QApplication::translate("TrackListPage", "Brian.Crain", nullptr));
        pageInfoLabel->setText(QApplication::translate("TrackListPage", "Playlist \302\267 17 songs, 11 hr 9 min 43 sec \302\267 Private", nullptr));
        pageInfo2Label->setText(QApplication::translate("TrackListPage", "TextLabel", nullptr));
        playButton->setText(QApplication::translate("TrackListPage", "Play", nullptr));
        insertToQueueButton->setText(QApplication::translate("TrackListPage", "Insert to queue", nullptr));
#ifndef QT_NO_TOOLTIP
        appendToQueueEndButton->setToolTip(QApplication::translate("TrackListPage", "Add to the End of Queue", nullptr));
#endif // QT_NO_TOOLTIP
        appendToQueueEndButton->setText(QApplication::translate("TrackListPage", "Append to queue", nullptr));
        favorButton->setText(QApplication::translate("TrackListPage", "Favor", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TrackListPage: public Ui_TrackListPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TrackListPage_H
