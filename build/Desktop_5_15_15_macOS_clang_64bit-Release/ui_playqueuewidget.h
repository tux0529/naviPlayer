/********************************************************************************
** Form generated from reading UI file 'playqueuewidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.15
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYQUEUEWIDGET_H
#define UI_PLAYQUEUEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <anchortableview.h>

QT_BEGIN_NAMESPACE

class Ui_PlayQueueWidget
{
public:
    QVBoxLayout *verticalLayout;
    AnchorTableView *tableView;
    QHBoxLayout *horizontalLayout;
    QLabel *infoLabel;
    QSpacerItem *horizontalSpacer;
    QToolButton *clearButton;
    QToolButton *playbackModeButton;
    QToolButton *moveUpButton;
    QToolButton *moveDownButton;
    QToolButton *removeButton;

    void setupUi(QWidget *PlayQueueWidget)
    {
        if (PlayQueueWidget->objectName().isEmpty())
            PlayQueueWidget->setObjectName(QString::fromUtf8("PlayQueueWidget"));
        PlayQueueWidget->resize(594, 459);
        PlayQueueWidget->setAutoFillBackground(true);
        verticalLayout = new QVBoxLayout(PlayQueueWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        tableView = new AnchorTableView(PlayQueueWidget);
        tableView->setObjectName(QString::fromUtf8("tableView"));

        verticalLayout->addWidget(tableView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        infoLabel = new QLabel(PlayQueueWidget);
        infoLabel->setObjectName(QString::fromUtf8("infoLabel"));

        horizontalLayout->addWidget(infoLabel);

        horizontalSpacer = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        clearButton = new QToolButton(PlayQueueWidget);
        clearButton->setObjectName(QString::fromUtf8("clearButton"));

        horizontalLayout->addWidget(clearButton);

        playbackModeButton = new QToolButton(PlayQueueWidget);
        playbackModeButton->setObjectName(QString::fromUtf8("playbackModeButton"));

        horizontalLayout->addWidget(playbackModeButton);

        moveUpButton = new QToolButton(PlayQueueWidget);
        moveUpButton->setObjectName(QString::fromUtf8("moveUpButton"));

        horizontalLayout->addWidget(moveUpButton);

        moveDownButton = new QToolButton(PlayQueueWidget);
        moveDownButton->setObjectName(QString::fromUtf8("moveDownButton"));

        horizontalLayout->addWidget(moveDownButton);

        removeButton = new QToolButton(PlayQueueWidget);
        removeButton->setObjectName(QString::fromUtf8("removeButton"));

        horizontalLayout->addWidget(removeButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(PlayQueueWidget);

        QMetaObject::connectSlotsByName(PlayQueueWidget);
    } // setupUi

    void retranslateUi(QWidget *PlayQueueWidget)
    {
        PlayQueueWidget->setWindowTitle(QCoreApplication::translate("PlayQueueWidget", "Form", nullptr));
        infoLabel->setText(QCoreApplication::translate("PlayQueueWidget", "\345\205\2617\351\246\226", nullptr));
        clearButton->setText(QCoreApplication::translate("PlayQueueWidget", "...", nullptr));
        playbackModeButton->setText(QCoreApplication::translate("PlayQueueWidget", "...", nullptr));
        moveUpButton->setText(QCoreApplication::translate("PlayQueueWidget", "...", nullptr));
        moveDownButton->setText(QCoreApplication::translate("PlayQueueWidget", "...", nullptr));
        removeButton->setText(QCoreApplication::translate("PlayQueueWidget", "...", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PlayQueueWidget: public Ui_PlayQueueWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYQUEUEWIDGET_H
