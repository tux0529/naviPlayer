/**
 * @file    songslistview.h
 * @brief   歌曲列表视图：分页、右键菜单、可选拖动排序（供歌曲页/专辑详情/歌单详情/缓存页复用）
 */
#ifndef NAVIPLAYER_SONGSLISTVIEW_H
#define NAVIPLAYER_SONGSLISTVIEW_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/song.h"

class SongTableModel;
class SongTableView;
class QLabel;
class QPushButton;

class SongsListView : public PageBase
{
    Q_OBJECT
public:
    explicit SongsListView(QWidget *parent = nullptr);

    void setSongs(const QVector<Song> &songs);
    QVector<Song> songs() const;
    void appendSongs(const QVector<Song> &songs);
    void clear();

    void setPagingEnabled(bool enabled);
    void setDragReorderEnabled(bool enabled);
    void setPlayingSongId(const QString &songId);
    void setFavorite(const QString &songId, bool favorite);
    /** 允许点击表头排序（缓存歌曲页按艺术家/专辑排序） */
    void setSortingEnabled(bool enabled);
    SongTableView *view() const { return m_view; }

    void refresh() override;
    void retranslate() override;

signals:
    void songActivated(const Song &song);
    void playNextRequested(const Song &song);
    void favoriteToggled(const Song &song, bool favorite);
    void addToPlaylistRequested(const Song &song, const QString &playlistId);
    void downloadRequested(const Song &song);
    void detailsRequested(const Song &song);
    /** 拖动排序后触发（歌单详情保存顺序用） */
    void orderChanged(const QVector<Song> &songs);

private slots:
    void onPrevPage();
    void onNextPage();
    void onModelSongsChanged();

private:
    void rebuildPage();
    void updatePageLabel();
    int songsPerPage() const;
    int pageCount() const;

    SongTableModel *m_model = nullptr;
    SongTableView *m_view = nullptr;
    QLabel *m_pageLabel = nullptr;
    QPushButton *m_prevButton = nullptr;
    QPushButton *m_nextButton = nullptr;

    QVector<Song> m_allSongs;
    int m_page = 0;
    bool m_pagingEnabled = true;
};

#endif // NAVIPLAYER_SONGSLISTVIEW_H
