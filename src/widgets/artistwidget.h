#ifndef ARTISTWIDGET_H
#define ARTISTWIDGET_H

#include <QWidget>
#include <QHash>
#include <QLineEdit>
#include <QMetaObject>
#include <QSet>
#include <QTableWidget>
#include <QVector>
#include <QShowEvent>
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
    void showEvent(QShowEvent *event) override;

private slots:
    void onCellDoubleClicked(int row, int column);
    void onCellClicked(int row, int column);
    void onSearchTextChanged(const QString &text);
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);
    void onVerticalScrollValueChanged(int value);

private:
    void setupUI();
    void rebuildTable();
    void updateFavoriteColumn();
    void updateResponsiveLayout();
    void requestVisibleCoverArts();
    void cancelPendingCoverArtRequests();
    Artist artistAtRow(int row) const;

    QLineEdit *m_searchEdit;
    QTableWidget *m_tableWidget;
    QVector<Artist> m_artists;
    QHash<QString, Artist> m_artistById;
    QSet<QString> m_favoriteArtistIds;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    QMetaObject::Connection m_coverArtConnection;
    // coverArtId -> "用于构建表时的原始 row"; 注意排序后视觉行号与 row 不同，
    // onCoverArtReceived 会通过 coverArtId 在表内反向查找行，因此可以跨排序使用。
    QHash<QString, int> m_coverArtRowMap;
    // 已通过 API 发起但尚未收到 coverArtReceived 回调的封面 id，用于去重。
    QSet<QString> m_pendingCoverArtIds;
};

#endif // ARTISTWIDGET_H
