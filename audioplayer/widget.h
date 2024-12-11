#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

namespace Ui {
class Widget;
}
class PlaybackQueue;
class AudioPlayer;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;

    AudioPlayer *m_player;
    PlaybackQueue *m_playbackQueue;

};

#endif // WIDGET_H
