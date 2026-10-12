/**
 * @file    albumcard.h
 * @brief   专辑卡片：封面（hover 播放按钮）+ 标题（居中单行/悬停滚动）+ 艺术家（可点击）
 */
#ifndef NAVIPLAYER_ALBUMCARD_H
#define NAVIPLAYER_ALBUMCARD_H

#include <QFrame>
#include <QString>

#include "core/qt_compat.h"
#include "models/album.h"
#include "models/artist.h"

class CoverLabel;
class QLabel;
class QPushButton;
class QTimer;

class AlbumCard : public QFrame
{
    Q_OBJECT
public:
    explicit AlbumCard(QWidget *parent = nullptr);

    void setAlbum(const Album &album, int coverSize = 160);
    Album album() const { return m_album; }
    void setCoverSize(int size);

signals:
    /** 点击封面或标题 */
    void albumClicked(const Album &album);
    void artistClicked(const Artist &artist);
    void playClicked(const Album &album);

protected:
    void enterEvent(NaviEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onScrollTick();

private:
    void updateTitle();
    /** 标题是否需要滚动（被截断） */
    bool titleTruncated() const;

    CoverLabel *m_cover = nullptr;
    QLabel *m_titleLabel = nullptr;
    QPushButton *m_artistButton = nullptr;
    QTimer *m_scrollTimer = nullptr;
    Album m_album;
    QString m_fullTitle;
    int m_coverSize = 160;
    int m_scrollPos = 0;
    bool m_hovered = false;
};

#endif // NAVIPLAYER_ALBUMCARD_H
