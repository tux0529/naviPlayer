#ifndef ARTISTWIDGET_H
#define ARTISTWIDGET_H

#include <QWidget>
#include <QHash>
#include <QLineEdit>
#include <QMetaObject>
#include <QSet>
#include <QTableWidget>
#include <QVector>
#include "../models/artist.h"
#include "../models/album.h"
#include "../models/song.h"

class CacheManager;
class SubsonicApi;

class ArtistWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArtistWidget(QWidget *parent = nullptr);
    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setArtists(const QVector<Artist> &artists);
    void setFavoriteArtistIds(const QSet<QString> &artistIds);
    void clear();

signals:
    void artistSelected(const Artist &artist);
    void artistFavoriteClicked(const Artist &artist);
    void artistSearchChanged(const QString &keyword);
    void artistSortChanged(int column, Qt::SortOrder order);
    void albumSelected(const Album &album);
    void songSelected(const Song &song);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCellDoubleClicked(int row, int column);
    void onCellClicked(int row, int column);
    void onSearchTextChanged(const QString &text);
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);

private:
    void setupUI();
    void rebuildTable();
    void updateFavoriteColumn();
    void updateResponsiveLayout();
    void requestVisibleCoverArts();
    Artist artistAtRow(int row) const;

    QLineEdit *m_searchEdit;
    QTableWidget *m_tableWidget;
    QVector<Artist> m_artists;
    QSet<QString> m_favoriteArtistIds;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    QMetaObject::Connection m_coverArtConnection;
    QHash<QString, int> m_coverArtRowMap;
};

#endif // ARTISTWIDGET_H
