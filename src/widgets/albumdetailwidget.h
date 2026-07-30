#ifndef ALBUMDETAILWIDGET_H
#define ALBUMDETAILWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QToolButton>
#include "../models/album.h"
#include "../models/song.h"

// Forward declarations
class SubsonicApi;
class CacheManager;
class SongListWidget;

class AlbumDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumDetailWidget(QWidget *parent = nullptr);
    
    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setAlbum(const Album &album, const QVector<Song> &songs);
    void clear();
    void setAlbumFavorite(bool favorite);

signals:
    void songSelected(const Song &song);
    void songFavoriteClicked(const Song &song);
    void albumFavoriteClicked(const Album &album);
    void playAlbum(const QVector<Song> &songs);
    void addSongsToPlaylist(const QVector<Song> &songs);
    void addAlbumToNext(const QVector<Song> &songs);
    void backRequested();

public slots:
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);

private slots:
    void onPlayClicked();
    void onAddToPlaylistClicked();
    void onAddToNextClicked();
    void onBackClicked();

private:
    void setupUI();
    void loadAlbumCover();

    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    Album m_currentAlbum;
    QVector<Song> m_songs;
    bool m_albumIsFavorite;

    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_infoLabel;
    QPushButton *m_backButton;
    QPushButton *m_playButton;
    QPushButton *m_addToPlaylistButton;
    QPushButton *m_addToNextButton;
    QToolButton *m_albumFavoriteButton;
    SongListWidget *m_songListWidget;
};

#endif // ALBUMDETAILWIDGET_H
