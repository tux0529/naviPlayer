#ifndef CARDWIDGET_H
#define CARDWIDGET_H

#include "album.h"
#include <QWidget>


namespace Ui {
class CardWidget;
}

class QLabel;
class FloatFrame;

class CardWidget : public QWidget
{
    Q_OBJECT

public:
    const static int width;

public:
    explicit CardWidget(const Album &album,
                        QWidget *parent = nullptr);
    ~CardWidget();

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void playIconClicked(const QString &albumId);
    void linkActivated(const QString &link);

private slots:
    void iconClicked();
private:
    Ui::CardWidget *ui;
    QLabel *m_label;
    FloatFrame *m_playFrame;

    Album m_album;
};

#endif // CARDWIDGET_H
