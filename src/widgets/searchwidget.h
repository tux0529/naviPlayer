#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QTabWidget>
#include <QLabel>
#include "../models/song.h"
#include "../models/album.h"
#include "../models/artist.h"
#include "../subsonicapi.h"
#include "../cachemanager.h"

class SongListWidget;
class AlbumWidget;
class ArtistWidget;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(QWidget *parent = nullptr);

    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);

signals:
    void songSelected(const Song &song);
    void songFavoriteClicked(const Song &song);
    void albumSelected(const Album &album);
    void artistSelected(const Artist &artist);

public slots:
    void onSearchResultsReceived(const QVector<Artist> &artists,
                                  const QVector<Album> &albums,
                                  const QVector<Song> &songs);

private slots:
    void onSearchTextChanged(const QString &text);

private:
    void setupUI();

    QLineEdit *m_searchInput;
    QTabWidget *m_resultsTab;
    SongListWidget *m_songResults;
    AlbumWidget *m_albumResults;
    ArtistWidget *m_artistResults;
    QLabel *m_noResultsLabel;

    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
};

#endif // SEARCHWIDGET_H
