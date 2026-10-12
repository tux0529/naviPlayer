/**
 * @file    artistdetailpage.h
 * @brief   艺术家详情页：头像 + 专辑数描述 + 收藏开关 + 专辑列表
 */
#ifndef NAVIPLAYER_ARTISTDETAILPAGE_H
#define NAVIPLAYER_ARTISTDETAILPAGE_H

#include "widgets/pagebase.h"

#include "models/artist.h"
#include "models/album.h"
#include "models/song.h"

class CoverLabel;
class QLabel;
class QPushButton;
class AlbumsView;

class ArtistDetailPage : public PageBase
{
    Q_OBJECT
public:
    explicit ArtistDetailPage(QWidget *parent = nullptr);

    void setArtist(const Artist &artist);
    Artist artist() const { return m_artist; }

    void refresh() override;
    void retranslate() override;

private:
    /** 按设备像素比请求头像，避免高分屏下被放大模糊 */
    void updateAvatar();

signals:
    void favoriteToggled(const Artist &artist, bool favorite);
    void albumClicked(const Album &album);
    void playAlbumRequested(const Album &album);

private:
    CoverLabel *m_avatar = nullptr;
    QLabel *m_nameLabel = nullptr;
    QLabel *m_descLabel = nullptr;
    QPushButton *m_favoriteButton = nullptr;
    AlbumsView *m_albumsView = nullptr;
    Artist m_artist;
};

#endif // NAVIPLAYER_ARTISTDETAILPAGE_H
