/**
 * @file    coverlabel.h
 * @brief   封面标签：异步加载封面（本地缓存优先），支持 hover 半透明播放按钮
 */
#ifndef NAVIPLAYER_COVERLABEL_H
#define NAVIPLAYER_COVERLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QString>

#include "core/qt_compat.h"

class CacheManager;

class CoverLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CoverLabel(QWidget *parent = nullptr);

    void setCacheManager(CacheManager *cache);
    void setCoverArt(const QString &coverArtId, int size);
    void setHoverPlayEnabled(bool enabled);
    void setPlaceholderText(const QString &text);
    void setRadius(int radius);
    void clear();

signals:
    /** 点击封面本体 */
    void clicked();
    /** 点击 hover 播放按钮 */
    void playClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(NaviEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onCoverReady(const QString &coverArtId, int size, const QPixmap &pixmap);

private:
    bool playButtonContains(const QPointF &pos) const;
    void drawPlaceholder(QPainter *painter);

    CacheManager *m_cache = nullptr;
    QString m_coverArtId;
    int m_requestedSize = 0;     // 逻辑显示尺寸
    int m_fetchSize = 0;         // 实际向缓存/服务端取样的尺寸（含设备像素比）
    QPixmap m_pixmap;
    QString m_placeholderText;
    int m_radius = 6;
    bool m_hoverPlayEnabled = false;
    bool m_hovered = false;
    bool m_connected = false;
};

#endif // NAVIPLAYER_COVERLABEL_H
