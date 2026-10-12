/**
 * @file    albumrowscroller.h
 * @brief   主页横向专辑行：单行专辑卡片 + 右上角左右滚动箭头
 *
 * 特性：
 *  - 卡片在单行内水平排列，超出可视宽度时横向滚动；
 *  - 左右箭头固定在右侧标题栏，始终可见、不随列表滚动；
 *  - 滚到最左禁用左箭头、最右禁用右箭头（基于滚动条 value）；
 *  - 点击箭头按“前一屏/后一屏”平滑滚动，支持触摸滑动与鼠标拖动；
 *  - 桌面鼠标滚轮映射为横向滚动。
 */
#ifndef NAVIPLAYER_ALBUMROWSCROLLER_H
#define NAVIPLAYER_ALBUMROWSCROLLER_H

#include <QWidget>
#include <QVector>

#include "models/album.h"
#include "models/artist.h"

class QLabel;
class QPushButton;
class QScrollArea;
class QHBoxLayout;
class QTimer;

class AlbumRowScroller : public QWidget
{
    Q_OBJECT
public:
    explicit AlbumRowScroller(QWidget *parent = nullptr);

    void setTitle(const QString &title);
    void setAlbums(const QVector<Album> &albums);
    void setCoverSize(int size);

signals:
    void albumClicked(const Album &album);
    void artistClicked(const Artist &artist);
    void playClicked(const Album &album);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void buildCards();

private:
    void clearCards();
    void scrollByPage(int direction);
    void updateArrowState();
    void animateScrollTo(int value);

    QLabel *m_title = nullptr;
    QPushButton *m_leftArrow = nullptr;
    QPushButton *m_rightArrow = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QHBoxLayout *m_rowLayout = nullptr;
    QTimer *m_buildTimer = nullptr;
    QVector<Album> m_albums;
    int m_coverSize = 152;      // 与 AlbumsView 的 cardWidth - 24 保持一致
};

#endif // NAVIPLAYER_ALBUMROWSCROLLER_H
