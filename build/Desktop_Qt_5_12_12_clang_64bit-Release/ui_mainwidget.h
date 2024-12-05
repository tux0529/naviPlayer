/********************************************************************************
** Form generated from reading UI file 'mainwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWIDGET_H
#define UI_MAINWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWidget
{
public:
    QVBoxLayout *shadowLayout;
    QWidget *shadowWidget;
    QVBoxLayout *Layout;
    QSplitter *splitter;
    QWidget *leftWidget;
    QVBoxLayout *verticalLayout_8;
    QHBoxLayout *horizontalLayout_6;
    QWidget *iconWidget;
    QVBoxLayout *verticalLayout_7;
    QLabel *iconLabel;
    QLabel *appNameLabel;
    QPushButton *homeButton;
    QPushButton *albumListButton;
    QPushButton *artistButton;
    QPushButton *genresButton;
    QPushButton *favoritesButton;
    QPushButton *randomSongsButton;
    QFrame *line;
    QPushButton *playlistsButton;
    QSpacerItem *verticalSpacer_2;
    QWidget *rightWidget;
    QVBoxLayout *verticalLayout;
    QWidget *titleWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *titleLabel;
    QWidget *menuWidget;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout;
    QToolButton *menuButton;
    QPushButton *minButton;
    QPushButton *maxButton;
    QPushButton *closeButton;
    QSpacerItem *verticalSpacer;
    QWidget *mainWidget;
    QVBoxLayout *verticalLayout_5;
    QStackedWidget *stackedWidget;
    QWidget *songsPage;
    QVBoxLayout *verticalLayout_3;
    QTableView *songsTableView;
    QWidget *bottomWidget;
    QHBoxLayout *horizontalLayout_5;
    QLabel *coverLabel;
    QVBoxLayout *verticalLayout_6;
    QLabel *currentSongTitleLabel;
    QLabel *currentSongAlbumLabel;
    QHBoxLayout *horizontalLayout_4;
    QLabel *progressLabel;
    QSlider *progressSlider;
    QLabel *durationLabel;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *prevButton;
    QPushButton *playButton;
    QPushButton *nextButton;
    QPushButton *favorButton;
    QPushButton *muteButton;
    QSlider *volumnSlider;
    QPushButton *playbackQueueButton;

    void setupUi(QWidget *MainWidget)
    {
        if (MainWidget->objectName().isEmpty())
            MainWidget->setObjectName(QString::fromUtf8("MainWidget"));
        MainWidget->resize(1366, 730);
        shadowLayout = new QVBoxLayout(MainWidget);
        shadowLayout->setSpacing(0);
        shadowLayout->setObjectName(QString::fromUtf8("shadowLayout"));
        shadowLayout->setContentsMargins(6, 6, 6, 6);
        shadowWidget = new QWidget(MainWidget);
        shadowWidget->setObjectName(QString::fromUtf8("shadowWidget"));
        Layout = new QVBoxLayout(shadowWidget);
        Layout->setSpacing(0);
        Layout->setObjectName(QString::fromUtf8("Layout"));
        Layout->setContentsMargins(1, 1, 1, 1);
        splitter = new QSplitter(shadowWidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(2);
        sizePolicy.setVerticalStretch(8);
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setOrientation(Qt::Horizontal);
        splitter->setOpaqueResize(true);
        splitter->setHandleWidth(1);
        splitter->setChildrenCollapsible(false);
        leftWidget = new QWidget(splitter);
        leftWidget->setObjectName(QString::fromUtf8("leftWidget"));
        leftWidget->setMinimumSize(QSize(250, 0));
        verticalLayout_8 = new QVBoxLayout(leftWidget);
        verticalLayout_8->setSpacing(0);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        verticalLayout_8->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        iconWidget = new QWidget(leftWidget);
        iconWidget->setObjectName(QString::fromUtf8("iconWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(iconWidget->sizePolicy().hasHeightForWidth());
        iconWidget->setSizePolicy(sizePolicy1);
        iconWidget->setMinimumSize(QSize(50, 50));
        iconWidget->setMaximumSize(QSize(50, 50));
        iconWidget->setAutoFillBackground(true);
        verticalLayout_7 = new QVBoxLayout(iconWidget);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        verticalLayout_7->setContentsMargins(6, 6, 6, 6);
        iconLabel = new QLabel(iconWidget);
        iconLabel->setObjectName(QString::fromUtf8("iconLabel"));
        iconLabel->setAlignment(Qt::AlignCenter);

        verticalLayout_7->addWidget(iconLabel);


        horizontalLayout_6->addWidget(iconWidget);

        appNameLabel = new QLabel(leftWidget);
        appNameLabel->setObjectName(QString::fromUtf8("appNameLabel"));

        horizontalLayout_6->addWidget(appNameLabel);


        verticalLayout_8->addLayout(horizontalLayout_6);

        homeButton = new QPushButton(leftWidget);
        homeButton->setObjectName(QString::fromUtf8("homeButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(homeButton->sizePolicy().hasHeightForWidth());
        homeButton->setSizePolicy(sizePolicy2);
        homeButton->setMinimumSize(QSize(0, 0));
        homeButton->setFlat(true);

        verticalLayout_8->addWidget(homeButton);

        albumListButton = new QPushButton(leftWidget);
        albumListButton->setObjectName(QString::fromUtf8("albumListButton"));
        albumListButton->setMinimumSize(QSize(0, 0));
        albumListButton->setFlat(true);

        verticalLayout_8->addWidget(albumListButton);

        artistButton = new QPushButton(leftWidget);
        artistButton->setObjectName(QString::fromUtf8("artistButton"));
        artistButton->setMinimumSize(QSize(0, 0));
        artistButton->setFlat(true);

        verticalLayout_8->addWidget(artistButton);

        genresButton = new QPushButton(leftWidget);
        genresButton->setObjectName(QString::fromUtf8("genresButton"));
        genresButton->setMinimumSize(QSize(0, 0));
        genresButton->setFlat(true);

        verticalLayout_8->addWidget(genresButton);

        favoritesButton = new QPushButton(leftWidget);
        favoritesButton->setObjectName(QString::fromUtf8("favoritesButton"));
        favoritesButton->setMinimumSize(QSize(0, 0));
        favoritesButton->setFlat(true);

        verticalLayout_8->addWidget(favoritesButton);

        randomSongsButton = new QPushButton(leftWidget);
        randomSongsButton->setObjectName(QString::fromUtf8("randomSongsButton"));
        randomSongsButton->setFlat(true);

        verticalLayout_8->addWidget(randomSongsButton);

        line = new QFrame(leftWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShadow(QFrame::Raised);
        line->setMidLineWidth(0);
        line->setFrameShape(QFrame::HLine);

        verticalLayout_8->addWidget(line);

        playlistsButton = new QPushButton(leftWidget);
        playlistsButton->setObjectName(QString::fromUtf8("playlistsButton"));
        playlistsButton->setMinimumSize(QSize(0, 0));
        playlistsButton->setFlat(true);

        verticalLayout_8->addWidget(playlistsButton);

        verticalSpacer_2 = new QSpacerItem(20, 315, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_8->addItem(verticalSpacer_2);

        splitter->addWidget(leftWidget);
        rightWidget = new QWidget(splitter);
        rightWidget->setObjectName(QString::fromUtf8("rightWidget"));
        sizePolicy2.setHeightForWidth(rightWidget->sizePolicy().hasHeightForWidth());
        rightWidget->setSizePolicy(sizePolicy2);
        rightWidget->setMinimumSize(QSize(0, 0));
        verticalLayout = new QVBoxLayout(rightWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        titleWidget = new QWidget(rightWidget);
        titleWidget->setObjectName(QString::fromUtf8("titleWidget"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(titleWidget->sizePolicy().hasHeightForWidth());
        titleWidget->setSizePolicy(sizePolicy3);
        titleWidget->setMinimumSize(QSize(0, 30));
        titleWidget->setMaximumSize(QSize(16777215, 30));
        horizontalLayout_2 = new QHBoxLayout(titleWidget);
        horizontalLayout_2->setSpacing(12);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(12, 0, 0, 0);
        titleLabel = new QLabel(titleWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(titleLabel);

        menuWidget = new QWidget(titleWidget);
        menuWidget->setObjectName(QString::fromUtf8("menuWidget"));
        verticalLayout_4 = new QVBoxLayout(menuWidget);
        verticalLayout_4->setSpacing(0);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        menuButton = new QToolButton(menuWidget);
        menuButton->setObjectName(QString::fromUtf8("menuButton"));

        horizontalLayout->addWidget(menuButton);

        minButton = new QPushButton(menuWidget);
        minButton->setObjectName(QString::fromUtf8("minButton"));
        minButton->setMinimumSize(QSize(30, 0));
        minButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(minButton);

        maxButton = new QPushButton(menuWidget);
        maxButton->setObjectName(QString::fromUtf8("maxButton"));
        maxButton->setMinimumSize(QSize(30, 0));
        maxButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(maxButton);

        closeButton = new QPushButton(menuWidget);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));
        closeButton->setMinimumSize(QSize(30, 0));
        closeButton->setMaximumSize(QSize(30, 16777215));

        horizontalLayout->addWidget(closeButton);


        verticalLayout_4->addLayout(horizontalLayout);

        verticalSpacer = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);


        horizontalLayout_2->addWidget(menuWidget);

        horizontalLayout_2->setStretch(0, 1);

        verticalLayout->addWidget(titleWidget);

        mainWidget = new QWidget(rightWidget);
        mainWidget->setObjectName(QString::fromUtf8("mainWidget"));
        verticalLayout_5 = new QVBoxLayout(mainWidget);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        verticalLayout_5->setContentsMargins(-1, -1, 0, -1);
        stackedWidget = new QStackedWidget(mainWidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        songsPage = new QWidget();
        songsPage->setObjectName(QString::fromUtf8("songsPage"));
        verticalLayout_3 = new QVBoxLayout(songsPage);
        verticalLayout_3->setSpacing(12);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        songsTableView = new QTableView(songsPage);
        songsTableView->setObjectName(QString::fromUtf8("songsTableView"));

        verticalLayout_3->addWidget(songsTableView);

        stackedWidget->addWidget(songsPage);

        verticalLayout_5->addWidget(stackedWidget);


        verticalLayout->addWidget(mainWidget);

        splitter->addWidget(rightWidget);

        Layout->addWidget(splitter);

        bottomWidget = new QWidget(shadowWidget);
        bottomWidget->setObjectName(QString::fromUtf8("bottomWidget"));
        sizePolicy3.setHeightForWidth(bottomWidget->sizePolicy().hasHeightForWidth());
        bottomWidget->setSizePolicy(sizePolicy3);
        bottomWidget->setMinimumSize(QSize(0, 80));
        horizontalLayout_5 = new QHBoxLayout(bottomWidget);
        horizontalLayout_5->setSpacing(9);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(12, 0, 12, 0);
        coverLabel = new QLabel(bottomWidget);
        coverLabel->setObjectName(QString::fromUtf8("coverLabel"));

        horizontalLayout_5->addWidget(coverLabel);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(6, -1, -1, -1);
        currentSongTitleLabel = new QLabel(bottomWidget);
        currentSongTitleLabel->setObjectName(QString::fromUtf8("currentSongTitleLabel"));

        verticalLayout_6->addWidget(currentSongTitleLabel);

        currentSongAlbumLabel = new QLabel(bottomWidget);
        currentSongAlbumLabel->setObjectName(QString::fromUtf8("currentSongAlbumLabel"));

        verticalLayout_6->addWidget(currentSongAlbumLabel);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(12);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        progressLabel = new QLabel(bottomWidget);
        progressLabel->setObjectName(QString::fromUtf8("progressLabel"));

        horizontalLayout_4->addWidget(progressLabel);

        progressSlider = new QSlider(bottomWidget);
        progressSlider->setObjectName(QString::fromUtf8("progressSlider"));
        progressSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(progressSlider);

        durationLabel = new QLabel(bottomWidget);
        durationLabel->setObjectName(QString::fromUtf8("durationLabel"));

        horizontalLayout_4->addWidget(durationLabel);


        verticalLayout_6->addLayout(horizontalLayout_4);


        horizontalLayout_5->addLayout(verticalLayout_6);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_3);

        prevButton = new QPushButton(bottomWidget);
        prevButton->setObjectName(QString::fromUtf8("prevButton"));
        prevButton->setMinimumSize(QSize(40, 40));
        prevButton->setMaximumSize(QSize(40, 40));

        horizontalLayout_5->addWidget(prevButton);

        playButton = new QPushButton(bottomWidget);
        playButton->setObjectName(QString::fromUtf8("playButton"));
        sizePolicy1.setHeightForWidth(playButton->sizePolicy().hasHeightForWidth());
        playButton->setSizePolicy(sizePolicy1);
        playButton->setMinimumSize(QSize(50, 50));
        playButton->setMaximumSize(QSize(50, 50));
        playButton->setCheckable(true);

        horizontalLayout_5->addWidget(playButton);

        nextButton = new QPushButton(bottomWidget);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        nextButton->setMinimumSize(QSize(40, 40));
        nextButton->setMaximumSize(QSize(40, 40));

        horizontalLayout_5->addWidget(nextButton);

        favorButton = new QPushButton(bottomWidget);
        favorButton->setObjectName(QString::fromUtf8("favorButton"));
        favorButton->setMinimumSize(QSize(40, 40));
        favorButton->setMaximumSize(QSize(40, 40));
        favorButton->setCheckable(true);

        horizontalLayout_5->addWidget(favorButton);

        muteButton = new QPushButton(bottomWidget);
        muteButton->setObjectName(QString::fromUtf8("muteButton"));
        muteButton->setMinimumSize(QSize(40, 40));
        muteButton->setMaximumSize(QSize(40, 40));
        muteButton->setCheckable(true);

        horizontalLayout_5->addWidget(muteButton);

        volumnSlider = new QSlider(bottomWidget);
        volumnSlider->setObjectName(QString::fromUtf8("volumnSlider"));
        volumnSlider->setMaximum(100);
        volumnSlider->setValue(100);
        volumnSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_5->addWidget(volumnSlider);

        playbackQueueButton = new QPushButton(bottomWidget);
        playbackQueueButton->setObjectName(QString::fromUtf8("playbackQueueButton"));
        playbackQueueButton->setMinimumSize(QSize(40, 40));
        playbackQueueButton->setMaximumSize(QSize(40, 40));
        playbackQueueButton->setCheckable(true);

        horizontalLayout_5->addWidget(playbackQueueButton);

        horizontalLayout_5->setStretch(1, 1);

        Layout->addWidget(bottomWidget);


        shadowLayout->addWidget(shadowWidget);


        retranslateUi(MainWidget);

        QMetaObject::connectSlotsByName(MainWidget);
    } // setupUi

    void retranslateUi(QWidget *MainWidget)
    {
        MainWidget->setWindowTitle(QApplication::translate("MainWidget", "MainWidget", nullptr));
        iconLabel->setText(QString());
        appNameLabel->setText(QApplication::translate("MainWidget", "NaviPlayer", nullptr));
        homeButton->setText(QApplication::translate("MainWidget", "  Home", nullptr));
        albumListButton->setText(QApplication::translate("MainWidget", "  Album", nullptr));
        artistButton->setText(QApplication::translate("MainWidget", "  Artists", nullptr));
        genresButton->setText(QApplication::translate("MainWidget", "  Genres", nullptr));
        favoritesButton->setText(QApplication::translate("MainWidget", "  Favorites", nullptr));
        randomSongsButton->setText(QApplication::translate("MainWidget", "  Random", nullptr));
        playlistsButton->setText(QApplication::translate("MainWidget", "  Playlists", nullptr));
        titleLabel->setText(QApplication::translate("MainWidget", "TextLabel", nullptr));
        menuButton->setText(QApplication::translate("MainWidget", "...", nullptr));
        minButton->setText(QApplication::translate("MainWidget", "M", nullptr));
        maxButton->setText(QApplication::translate("MainWidget", "M", nullptr));
        closeButton->setText(QApplication::translate("MainWidget", "E", nullptr));
        coverLabel->setText(QApplication::translate("MainWidget", "Cover", nullptr));
        currentSongTitleLabel->setText(QString());
        currentSongAlbumLabel->setText(QString());
        progressLabel->setText(QApplication::translate("MainWidget", "0:00", nullptr));
        durationLabel->setText(QApplication::translate("MainWidget", "0:00", nullptr));
#ifndef QT_NO_TOOLTIP
        prevButton->setToolTip(QApplication::translate("MainWidget", "Previous", nullptr));
#endif // QT_NO_TOOLTIP
        prevButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
#ifndef QT_NO_TOOLTIP
        playButton->setToolTip(QApplication::translate("MainWidget", "Play / Pause", nullptr));
#endif // QT_NO_TOOLTIP
        playButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
#ifndef QT_NO_TOOLTIP
        nextButton->setToolTip(QApplication::translate("MainWidget", "Next", nullptr));
#endif // QT_NO_TOOLTIP
        nextButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
#ifndef QT_NO_TOOLTIP
        favorButton->setToolTip(QApplication::translate("MainWidget", "Favorite", nullptr));
#endif // QT_NO_TOOLTIP
        favorButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
#ifndef QT_NO_TOOLTIP
        muteButton->setToolTip(QApplication::translate("MainWidget", "Mute", nullptr));
#endif // QT_NO_TOOLTIP
        muteButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
#ifndef QT_NO_TOOLTIP
        playbackQueueButton->setToolTip(QApplication::translate("MainWidget", "Playback Queue", nullptr));
#endif // QT_NO_TOOLTIP
        playbackQueueButton->setText(QApplication::translate("MainWidget", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWidget: public Ui_MainWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWIDGET_H
