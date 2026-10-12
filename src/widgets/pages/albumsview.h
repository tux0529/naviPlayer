/**
 * @file    albumsview.h
 * @brief   专辑宫格视图（支持分页、来源切换：全部 / 收藏 / 某艺术家）
 */
#ifndef NAVIPLAYER_ALBUMSVIEW_H
#define NAVIPLAYER_ALBUMSVIEW_H

#include "widgets/pagebase.h"

#include <QVector>
#include <QDateTime>

#include "models/album.h"
#include "models/artist.h"

class QGridLayout;
class QLabel;
class QPushButton;
class AlbumCard;

class AlbumsView : public PageBase
{
    Q_OBJECT
public:
    enum class Source { All, Favorite, Artist };
    Q_ENUM(Source)

    explicit AlbumsView(QWidget *parent = nullptr);

    void setSource(Source source);
    Source source() const { return m_source; }
    void setArtist(const Artist &artist);

    /** 直接设置数据（搜索结果等外部数据源使用） */
    void setAlbums(const QVector<Album> &albums);

    void refresh() override;
    void retranslate() override;

signals:
    void albumClicked(const Album &album);
    void artistClicked(const Artist &artist);
    void playAlbumRequested(const Album &album);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onPrevPage();
    void onNextPage();

private:
    void loadLocal();
    void requestRemote();
    void rebuildPage();
    void updatePageLabel();
    int pageCount() const;
    int albumsPerPage() const;
    void clearCards();

    Source m_source = Source::All;
    Artist m_artist;
    QVector<Album> m_albums;
    int m_page = 0;

    QGridLayout *m_grid = nullptr;
    QWidget *m_gridWidget = nullptr;
    QLabel *m_pageLabel = nullptr;
    QPushButton *m_prevButton = nullptr;
    QPushButton *m_nextButton = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QVector<AlbumCard *> m_cards;
    QDateTime m_lastRemoteFetch;
    bool m_loading = false;
};

#endif // NAVIPLAYER_ALBUMSVIEW_H
