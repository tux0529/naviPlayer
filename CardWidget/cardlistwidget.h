#ifndef CARDLISTWIDGET_H
#define CARDLISTWIDGET_H

#include <QWidget>

namespace Ui {
class CardListWidget;
}

class CardWidget;
class QScrollBar;
class QPropertyAnimation;

class CardListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardListWidget(QWidget *parent = nullptr);
    ~CardListWidget();

    void setTitle(const QString &title);
    void addCard(CardWidget *card);

private slots:
    void onRightShift();
    void onLeftShift();

    void adjustButtonState();

private:

    Ui::CardListWidget *ui;
    QScrollBar *m_scrollBar;
    QPropertyAnimation *m_animation;

    const static int s_cardSpace;
    const static int s_cardWidth;
};

#endif // CARDLISTWIDGET_H
