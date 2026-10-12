/**
 * @file    cachedsongspage.h
 * @brief   本地缓存歌曲页：可删除、清空，可按艺术家/专辑排序
 */
#ifndef NAVIPLAYER_CACHEDSONGSPAGE_H
#define NAVIPLAYER_CACHEDSONGSPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/song.h"

class QLabel;
class QPushButton;
class SongsListView;

class CachedSongsPage : public PageBase
{
    Q_OBJECT
public:
    explicit CachedSongsPage(QWidget *parent = nullptr);

    void refresh() override;
    void retranslate() override;

signals:
    void playAllRequested(const QVector<Song> &songs);
    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);

private slots:
    void onPlayAll();
    void onDeleteSelected();
    void onClearAll();

private:
    void updateSummary(qint64 totalBytes);

    SongsListView *m_songsView = nullptr;
    QLabel *m_summaryLabel = nullptr;
    QPushButton *m_playAllButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QPushButton *m_clearButton = nullptr;
};

#endif // NAVIPLAYER_CACHEDSONGSPAGE_H
