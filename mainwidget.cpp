#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "iconhelper.h"
#include "mediahelper.h"

#include "playbackqueue.h"
#include "homepage.h"
#include "albumlistpage.h"
#include "artistspage.h"
#include "favoritespage.h"

#include "playqueuewidget.h"
#include "playlistspage.h"
#include "albuminfopage.h"
#include "playlistinfopage.h"
#include "artistinfopage.h"

#include "genrelistpage.h"

#include <QDir>
#include <QFile>
#include <QEvent>
#include <QMouseEvent>

#include <QMediaPlayer>

#include <QPixmap>
#include <QPixmapCache>


#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QDesktopWidget>
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
#include <QGraphicsDropShadowEffect>
#endif

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , m_genreListPage (new GenreListPage(this))
    , m_artistPage (new ArtistsPage(this))
    , m_homePage (new HomePage(this))
    , m_albumListPage (new AlbumListPage(this))
    , m_favoritesPage (new FavoritesPage(this))
    , m_playlistsPage (new PlaylistsPage(this))
    , m_albumInfoPage (new AlbumInfoPage(this))
    , m_playlistInfoPage (new PlaylistInfoPage(this))
    , m_artistInfoPage (new ArtistInfoPage(this))
    , m_playbackQueue(new PlaybackQueue(this))
    , m_playbackWidget(new PlayQueueWidget(m_playbackQueue, this))
    , m_player(new AudioPlayer(this))
    , m_duration(0)
{
    ui->setupUi(this);
    this->installEventFilter(this);

    qRegisterMetaType<AudioPlayer::MediaStatus>("AudioPlayer::MediaStatus");
    qRegisterMetaType<AudioPlayer::State>("AudioPlayer::State");
    initForm();
    m_bChangeProgresSlider = false;
}

MainWidget::~MainWidget()
{
    delete ui;
}

bool MainWidget::eventFilter(QObject *obj, QEvent *evt)
{
    static QPoint mousePoint;
    static bool mousePressed = false;

    switch (evt->type())
    {
    case QEvent::WindowTitleChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            ui->titleLabel->setText(pWidget->windowTitle());
            return true;
        }
        break;
    }
    case QEvent::WindowIconChange:
    {
        QWidget *pWidget = qobject_cast<QWidget *>(obj);
        if (pWidget)
        {
            QIcon icon = pWidget->windowIcon();
            ui->iconLabel->setPixmap(icon.pixmap(ui->iconLabel->size()));
            return true;
        }
        break;
    }
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton ){
            if (ui->titleWidget->geometry().contains( (event->pos() - static_cast<QWidget *>(ui->titleWidget->parent())->pos() ) )) {
                mousePressed = true;
                mousePoint = event->globalPos() - this->pos();
                return true;
            }

            if (m_playbackWidget->isVisible() && !m_playbackWidget->geometry().contains( (event->pos() ) )) {
                m_playbackWidget->hide();
                ui->playbackQueueButton->setChecked(false);
            }
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (event->button() == Qt::LeftButton ){
            mousePressed = false;
            if (m_playbackWidget->isVisible() && !m_playbackWidget->geometry().contains( (event->pos() ) )) {
                m_playbackWidget->hide();
                ui->playbackQueueButton->setChecked(false);
                return true;
            }
        }
        break;
    }
    case QEvent::MouseMove:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if ((!this->isMaximized()) && mousePressed && (event->buttons() == Qt::LeftButton)) {
            this->move(event->globalPos() - mousePoint);
            return true;
        }
        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        QMouseEvent *event = static_cast<QMouseEvent *>(evt);
        if (ui->titleWidget->geometry().contains(event->pos())) {
            this->on_maxButton_clicked();
            return true;
        }
        break;
    }
    case QEvent::Resize:
    {
        adjustWidgetSize();
        return true;
    }
    default:
    {

    }
    }
    return QWidget::eventFilter(obj, evt);
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    //Q_UNUSED(event);
    //m_player->stop();
    hide();
    //event->ignore();
}

void MainWidget::initForm()
{
    resize(1366,800);

    //Window
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

    setAttribute(Qt::WA_TranslucentBackground);

#if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0,0);
    shadow->setColor(QColor(0,0,0));
    shadow->setBlurRadius(9);
    ui->shadowWidget->setGraphicsEffect(shadow);
#endif

    //Title bar
    ui->iconWidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->iconLabel->setAttribute(Qt::WA_TranslucentBackground);
    ui->appNameLabel->setAttribute(Qt::WA_TranslucentBackground);
    ui->progressSlider->setAttribute(Qt::WA_TranslucentBackground);
    ui->volumnSlider->setAttribute(Qt::WA_TranslucentBackground);
    ui->titleLabel->setFont(QFont("Microsoft Yahei", 12));
    this->setWindowIcon(QIcon(":/images/Musicpl.png"));
    this->setWindowTitle(tr("NaviPlayer"));

    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 7);

    IconHelper::Instance()->setIcon(ui->menuButton, QChar(0xf0d7));
    IconHelper::Instance()->setIcon(ui->minButton, QChar(0xF2d1));
    IconHelper::Instance()->setIcon(ui->maxButton, QChar(0xF2d0));
    IconHelper::Instance()->setIcon(ui->closeButton, QChar(0xF00d));

    IconHelper::Instance()->setIcon(ui->prevButton, QChar(0xF048), 20);
    IconHelper::Instance()->setIcon(ui->playButton, QChar(0xF144), 36);
    IconHelper::Instance()->setIcon(ui->nextButton, QChar(0xF051), 20);
    IconHelper::Instance()->setIcon(ui->favorButton, QChar(0xF08a), 20);
    IconHelper::Instance()->setIcon(ui->muteButton, QChar(0xF028), 20);
    IconHelper::Instance()->setIcon(ui->playbackQueueButton, QChar(0xF0c9), 20);


    //Init PlayBack Queue Page
    m_playbackWidget->setObjectName(QString::fromUtf8("playbackWidget"));
    m_playbackWidget->hide();
    connect(m_playbackQueue, SIGNAL(currentTrackChanged(const Track &)), this, SLOT(onCurrentTrackChanged(const Track &)) );
    connect(m_playbackWidget, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkClicked(const QString &)));


    //Init Home Page
    m_homePage->setObjectName(QString::fromUtf8("homePage"));
    connect(m_homePage, SIGNAL(linkClicked(const QString &)), this, SLOT(onLinkClicked(const QString &)));
    connect(m_homePage, SIGNAL(playAlbum(const QString &)), this, SLOT(onPlayAlbum(const QString &)));
    ui->stackedWidget->insertWidget(0, m_homePage);

    //Init Album List Page
    m_albumListPage->setObjectName(QString::fromUtf8("albumListPage"));
    connect(m_albumListPage, SIGNAL(playIconClicked(const QString &)), this, SLOT(onPlayAlbum(const QString &)));
    ui->stackedWidget->insertWidget(1, m_albumListPage);

    //Init artist List Page
    m_artistPage->setObjectName(QString::fromUtf8("artistPage"));
    //connect(m_homePage, SIGNAL(linkClicked(const QString &)), this, SLOT(onLinkClicked(const QString &)));
    ui->stackedWidget->insertWidget(2, m_artistPage);

    //Init genre List Page
    m_genreListPage->setObjectName(QString::fromUtf8("genreListPage"));
    //connect(m_homePage, SIGNAL(linkClicked(const QString &)), this, SLOT(onLinkClicked(const QString &)));
    ui->stackedWidget->insertWidget(3, m_genreListPage);

    //Init Favorites Page
    m_favoritesPage->setObjectName(QString::fromUtf8("favoritesPage"));
    //connect(m_favoritesPage, SIGNAL(linkClicked(const QString &)), this, SLOT(onLinkClicked(const QString &)));
    ui->stackedWidget->insertWidget(4, m_favoritesPage);

    //Init Playlists Page
    m_playlistsPage->setObjectName(QString::fromUtf8("playlistsPage"));
    ui->stackedWidget->insertWidget(5, m_playlistsPage);
    connect(m_playlistsPage, SIGNAL(rowDoubleClicked(const QString &)), this, SLOT(onPlaylistDClicked(const QString &)));

    //Init Album Info Page
    m_albumInfoPage->setObjectName(QString::fromUtf8("albumInfoPage"));
    ui->stackedWidget->insertWidget(6, m_albumInfoPage);

    //Init PlaylistInfo Page
    m_playlistInfoPage->setObjectName(QString::fromUtf8("playlistInfoPage"));
    ui->stackedWidget->insertWidget(7, m_playlistInfoPage);

    //Init ArtistInfo Page
    m_artistInfoPage->setObjectName(QString::fromUtf8("artistInfoPage"));
    connect(m_artistInfoPage, SIGNAL(linkClicked(const QString &)), this, SLOT(onLinkClicked(const QString &)));
    connect(m_artistInfoPage, SIGNAL(playAlbum(const QString &)), this, SLOT(onPlayAlbum(const QString &)));
    ui->stackedWidget->insertWidget(8, m_artistInfoPage);

    //设置左侧导航按钮
    QList<QPushButton *> btns = ui->leftWidget->findChildren<QPushButton *>();
    foreach (QPushButton *btn, btns) {
        btn->setCheckable(true);
        setLeftNavButtonIcon(btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(leftButtonClick()));
    }

    m_player->setPlaybackQueue(m_playbackQueue);

    connect(ui->currentSongTitleLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkClicked(const QString &)) );
    connect(ui->currentSongAlbumLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(onLinkClicked(const QString &)) );

    connect(ui->playButton, &QPushButton::clicked, this, &MainWidget::onPlayButtonClicked);
    connect(ui->nextButton, SIGNAL(clicked()), m_playbackQueue, SLOT(next()) );
    connect(ui->prevButton, SIGNAL(clicked()), m_playbackQueue, SLOT(previous()) );

    connect(ui->progressSlider, SIGNAL(sliderPressed()), this, SLOT(onProgressSliderPressed()) );
    connect(ui->progressSlider, SIGNAL(sliderReleased()), this, SLOT(onChangeProgress()) );
    connect(ui->progressSlider, SIGNAL(sliderMoved(int)), this, SLOT(onChangeProgress()) );

    connect(ui->volumnSlider, SIGNAL(sliderReleased()), this, SLOT(onChangeVolume()) );
    connect(ui->volumnSlider, SIGNAL(sliderMoved(int)), this, SLOT(onChangeVolume()) );

    connect(ui->playbackQueueButton, SIGNAL(toggled(bool)), this, SLOT(playbackQueueButtonToggled(bool)) );

    connect(m_player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    connect(m_player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(m_player, SIGNAL(mediaStatusChanged(AudioPlayer::MediaStatus)), this, SLOT(mediaStatusChanged(AudioPlayer::MediaStatus)));
    connect(m_player, SIGNAL(stateChanged(AudioPlayer::State)), this, SLOT(stateChanged(AudioPlayer::State)));
    connect(m_player, SIGNAL(volumeChanged(int)), this, SLOT(volumeChanged(int)));

    //connect(m_playlist, SIGNAL(currentIndexChanged(int)), m_player, SLOT(play()));

    //connect(m_playlistsPage, SIGNAL(rowDoubleClicked(const SongQuery &)), this, SLOT(showSongList(const SongQuery &)));
    //connect(m_albumInfoPage, SIGNAL(rowDoubleClicked(SongList)), this, SLOT(addSongsToQueue(SongList)));

    ui->stackedWidget->setCurrentIndex(0);

    ui->coverLabel->resize(60,60);
    ui->coverLabel->setPixmap(QPixmap(":/images/Musicpl.png").scaled(60,60,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}


void MainWidget::on_closeButton_clicked()
{
    close();
}


void MainWidget::on_maxButton_clicked()
{
    if(this->isMaximized ()){
        ui->shadowLayout->setMargin(6);
        this->showNormal();
    }
    else{
        ui->shadowLayout->setMargin(0);
        this->showMaximized();
    }
}


void MainWidget::on_minButton_clicked()
{
    showMinimized();
}

void MainWidget::leftButtonClick()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    QString name = b->objectName();

    QList<QPushButton *> btns = ui->leftWidget->findChildren<QPushButton *>();
    foreach (QPushButton *btn, btns) {
        if (btn == b) {
            btn->setChecked(true);
        } else {
            btn->setChecked(false);
        }
    }

    if ( name == "homeButton"){
        ui->stackedWidget->setCurrentIndex(0);
    } else if ( name == "albumListButton"){
        ui->stackedWidget->setCurrentWidget(m_albumListPage);
    } else if ( name == "genresButton"){
        ui->stackedWidget->setCurrentWidget(m_genreListPage);
    } else if ( name == "artistButton"){
        ui->stackedWidget->setCurrentWidget(m_artistPage);
    } else if ( name == "favoritesButton"){
        ui->stackedWidget->setCurrentWidget(m_favoritesPage);
    } else if ( name == "tracksButton"){
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "playlistsButton") {
        ui->stackedWidget->setCurrentWidget(m_playlistsPage);
    }
}

void MainWidget::positionChanged(qint64 pos)
{
    if (m_bChangeProgresSlider)
        return;

    int val = pos / 1000;
    ui->progressSlider->setValue(val);
    ui->progressLabel->setText(Config::msecsToTimeString(pos));
}

void MainWidget::durationChanged(qint64 duration)
{
    this->m_duration = duration;
    ui->durationLabel->setText(Config::msecsToTimeString(duration));
    ui->progressSlider->setMaximum(duration/1000);
}

void MainWidget::volumeChanged(int volume)
{
    ui->volumnSlider->setValue(volume);
}

void MainWidget::onProgressSliderPressed(){
    m_bChangeProgresSlider = true;
}

void MainWidget::onChangeProgress()
{
    qDebug() << "MainWidget::onChangeProgress:" << ui->progressSlider->value();
    m_player->setPosition(ui->progressSlider->value() * 1000);
    m_bChangeProgresSlider = false;
}

void MainWidget::onChangeVolume()
{
    m_player->setVolume(ui->volumnSlider->value());
}

void MainWidget::mediaStatusChanged(AudioPlayer::MediaStatus status)
{
    qDebug() << "mediaStatusChanged:" << status;
    if(status == AudioPlayer::BufferedMedia){
    } else if(status == AudioPlayer::InvalidMedia){
    } else if(status == AudioPlayer::EndOfMedia){
    } else if(status == AudioPlayer::LoadedMedia){
    }
}

void MainWidget::stateChanged(AudioPlayer::State state)
{
    qDebug() << "MainWidget::stateChanged:" << state;
    if(AudioPlayer::PlayingState == state){
        IconHelper::Instance()->setIcon(ui->playButton, QChar(0xF28b), 36);
        ui->playButton->setChecked(true);
    } else {
        IconHelper::Instance()->setIcon(ui->playButton, QChar(0xF144), 36);
        ui->playButton->setChecked(false);
    }
}

void MainWidget::onPlayButtonClicked()
{
    //m_player->pause();

    qDebug() << "MainWidget::onPlayButtonClicked:" << m_player->state();

    //return;

    if ( m_player->state() == AudioPlayer::PlayingState){
        m_player->pause();
        return;
    }

    m_player->play();

}

void MainWidget::onPlaylistDClicked(const QString &id)
{
    m_playlistInfoPage->loadPlaylist(id);
    ui->stackedWidget->setCurrentWidget(m_playlistInfoPage);
}

void MainWidget::onLinkClicked(const QString &link)
{
    qDebug() << "handleLinkClicked" << link;

    if(!link.startsWith("search://"))
        return;

    QStringList sl = link.mid(9).split("=");
    if (sl.size() < 2)
        return;

    if (sl.at(0) == "artist")
        visitArtist(sl.at(1));
    else if (sl.at(0) == "album")
        visitAlbum(sl.at(1));
}

void MainWidget::onPlayAlbum(const QString &albumId)
{
    Config::G_Debug("MainWidget::onPlayAlbum: ", albumId);
    m_playbackQueue->addTracks(MediaHelper::Instance()->getAlbum(albumId).tracks());
}

void MainWidget::onCurrentTrackChanged(const Track &tk){
    if (tk.id() == 0)
        return;

    ui->currentSongTitleLabel->setText(tk.name());
    ui->currentSongAlbumLabel->setText(QString("<a style='color: #dcdcdc;' href='search://artist=%1'>%3</a> - <a style='color: #dcdcdc;' href='search://album=%2'>%4</a>")
                                           .arg(tk.artistId(), tk.albumId(), tk.artist(), tk.album()));


    ui->coverLabel->setPixmap(QPixmap::fromImage(QImage(MediaHelper::Instance()->getCoverArt(tk.albumId(), MediaHelper::AlbumCover, MediaHelper::Poster))
                                                     .scaled(ui->coverLabel->width(),ui->coverLabel->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));

    this->setWindowTitle(QString("%1 - %2").arg(tk.artist(), tk.name()));

}

void MainWidget::playbackQueueButtonToggled(bool checked)
{
    if (!checked){
        m_playbackWidget->hide();
    } else {
        adjustWidgetSize();
        m_playbackWidget->show();
        m_playbackWidget->raise();
    }
}

void MainWidget::adjustWidgetSize()
{
    QRect ret = ui->mainWidget->geometry();
    m_playbackWidget->resize(ret.width() / 2 , ret.height() -20);
    m_playbackWidget->move(ui->leftWidget->width() + ret.x() + ret.width() / 2 -20 , ret.y() + 20 );
}

void MainWidget::setLeftNavButtonIcon(QPushButton *btn)
{
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/theme/black/nav_%1.png").arg(btn->objectName()), QSize(), QIcon::Normal, QIcon::Off);
    btn->setIcon(icon);
}

void MainWidget::visitArtist(const QString &artist)
{
    m_artistInfoPage->loadArtist(artist);
    ui->stackedWidget->setCurrentWidget(m_artistInfoPage);
}

void MainWidget::visitAlbum(const QString &albumId)
{
    m_albumInfoPage->loadAlbum(albumId);
    ui->stackedWidget->setCurrentWidget(m_albumInfoPage);
}


void MainWidget::on_favorButton_clicked()
{

}

void MainWidget::showSongList()
{
    if (1){
        ui->stackedWidget->setCurrentWidget(m_albumInfoPage);
    }
}

void MainWidget::addSongsToQueue(TrackList tracklist)
{
    m_playbackQueue->addTracks(tracklist);
    if(m_player->state() == AudioPlayer::StoppedState){
        m_player->play();
    }
}

