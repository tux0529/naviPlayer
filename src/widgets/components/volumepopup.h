/**
 * @file    volumepopup.h
 * @brief   音量弹出面板（点击控制栏音量按钮弹出）
 */
#ifndef NAVIPLAYER_VOLUMEPOPUP_H
#define NAVIPLAYER_VOLUMEPOPUP_H

#include <QFrame>

class QSlider;
class QLabel;

class VolumePopup : public QFrame
{
    Q_OBJECT
public:
    explicit VolumePopup(QWidget *parent = nullptr);

    int volume() const;
    void setVolume(int volume);

signals:
    void volumeChanged(int volume);

private:
    QSlider *m_slider = nullptr;
    QLabel *m_label = nullptr;
};

#endif // NAVIPLAYER_VOLUMEPOPUP_H
