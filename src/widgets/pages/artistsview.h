/**
 * @file    artistsview.h
 * @brief   艺术家列表（头像、专辑数、收藏开关，双击进入详情）
 */
#ifndef NAVIPLAYER_ARTISTSVIEW_H
#define NAVIPLAYER_ARTISTSVIEW_H

#include "widgets/pagebase.h"

#include <QAbstractTableModel>
#include <QVector>
#include <QDateTime>

#include "models/artist.h"

class QTableView;
class QSortFilterProxyModel;

class ArtistTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column { ColAvatar = 0, ColName, ColAlbumCount, ColFavorite, ColCount };
    enum Roles { ArtistRole = Qt::UserRole + 1, CoverArtIdRole = Qt::UserRole + 2 };

    explicit ArtistTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setArtists(const QVector<Artist> &artists);
    QVector<Artist> artists() const { return m_artists; }
    Artist artistAt(int row) const;
    void setFavorite(const QString &artistId, bool favorite);

private:
    QVector<Artist> m_artists;
};

class ArtistsView : public PageBase
{
    Q_OBJECT
public:
    enum class Source { All, Favorite };
    Q_ENUM(Source)

    explicit ArtistsView(QWidget *parent = nullptr);
    void setSource(Source source);

    /** 直接设置数据（搜索结果等外部数据源使用） */
    void setArtists(const QVector<Artist> &artists);

    void refresh() override;
    void retranslate() override;

signals:
    void artistClicked(const Artist &artist);
    void favoriteToggled(const Artist &artist, bool favorite);

private slots:
    void onDoubleClicked(const QModelIndex &index);
    void onClicked(const QModelIndex &index);

private:
    void loadLocal();
    void requestRemote();

    ArtistTableModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxy = nullptr;
    QTableView *m_view = nullptr;
    Source m_source = Source::All;
    QDateTime m_lastRemoteFetch;
    bool m_loading = false;
};

#endif // NAVIPLAYER_ARTISTSVIEW_H
