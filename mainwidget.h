#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "config.h"
#include "track.h"
#include "audioplayer.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWidget; }
QT_END_NAMESPACE

class QPushButton;

class PlayQueueWidget;
class PlaylistsPage;
class AlbumInfoPage;
class PlaylistInfoPage;

class PlaybackQueue;

class HomePage;
class ArtistsPage;
class GenreListPage;
class AlbumListPage;
class FavoritesPage;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

protected:
    bool eventFilter(QObject *obj, QEvent *evt);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_closeButton_clicked();

    void on_maxButton_clicked();

    void on_minButton_clicked();

    void leftButtonClick();

    void positionChanged(qint64 pos);

    void durationChanged(qint64 duration);

    void volumeChanged(int volume);

    void onProgressSliderPressed();

    void onChangeProgress();

    void onChangeVolume();

    void mediaStatusChanged(AudioPlayer::MediaStatus status);

    void stateChanged(AudioPlayer::State state);

    void onPlayButtonClicked();

    void onPlaylistDClicked(const QString &id);

    void onLinkClicked(const QString &link);

    void onPlayAlbum(const QString &albumId);

    void playbackQueueButtonToggled(bool checked);

    void onCurrentTrackChanged(const Track &tk);

    void on_favorButton_clicked();

    void showSongList();

    void addSongsToQueue(TrackList tracklist);

    //void albumVisited(const QString &link);

private:
    Ui::MainWidget *ui;
    void initForm();
    void adjustWidgetSize();
    void setLeftNavButtonIcon(QPushButton *btn);


    GenreListPage *m_genreListPage;
    ArtistsPage *m_artistPage;
    HomePage *m_homePage;
    AlbumListPage *m_albumListPage;
    FavoritesPage *m_favoritesPage;
    PlaylistsPage *m_playlistsPage;

    AlbumInfoPage *m_albumInfoPage;
    PlaylistInfoPage *m_playlistInfoPage;

    PlaybackQueue *m_playbackQueue;
    PlayQueueWidget *m_playbackWidget;

    AudioPlayer *m_player;

    qint64 m_duration;

    void visitArtist(const QString &artist);
    void visitAlbum(const QString &albumId);

    bool m_bChangeProgresSlider;

};
#endif // MAINWIDGET_H
