#ifndef CARDWIDGET_H
#define CARDWIDGET_H

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
    explicit CardWidget(const QString &albumId,
                        const QPixmap &icoPixmap,
                        const QString &title,
                        const QString &artistName = QString(),
                        QWidget *parent = nullptr);
    ~CardWidget();

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void playIconClicked(const QString &albumId);
    void linkClicked(const QString &link);

private slots:
    void iconClicked();
private:
    Ui::CardWidget *ui;
    QLabel *m_label;
    FloatFrame *m_playFrame;

    QString m_albumId;
};

#endif // CARDWIDGET_H
