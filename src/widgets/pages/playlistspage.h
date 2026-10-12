/**
 * @file    playlistspage.h
 * @brief   全部播放列表：新建 / 重命名 / 删除，双击进入详情
 */
#ifndef NAVIPLAYER_PLAYLISTSPAGE_H
#define NAVIPLAYER_PLAYLISTSPAGE_H

#include "widgets/pagebase.h"

#include <QVector>

#include "models/playlist.h"

class QListWidget;
class QPushButton;

class PlaylistsPage : public PageBase
{
    Q_OBJECT
public:
    explicit PlaylistsPage(QWidget *parent = nullptr);

    void refresh() override;
    void retranslate() override;

signals:
    void playlistOpened(const Playlist &playlist);

private slots:
    void onNewPlaylist();
    void onRenamePlaylist();
    void onDeletePlaylist();
    void onItemDoubleClicked();

private:
    void updateList(const QVector<Playlist> &playlists);
    QString currentPlaylistId() const;

    QListWidget *m_list = nullptr;
    QPushButton *m_newButton = nullptr;
    QPushButton *m_renameButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QVector<Playlist> m_playlists;
};

#endif // NAVIPLAYER_PLAYLISTSPAGE_H
