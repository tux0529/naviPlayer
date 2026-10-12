#ifndef NAVWIDGET_H
#define NAVWIDGET_H

#include <QWidget>

namespace Ui {
class NavWidget;
}

class QPushButton;

class NavWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavWidget(QWidget *parent = nullptr);
    ~NavWidget();

    /** 切换主题图标（dark / light） */
    void applyTheme(bool dark);

signals:
    void buttonClicked(QPushButton *btn);

private slots:

    void buttonClick();

private:
    void setButtonIcon(QPushButton *btn);

    Ui::NavWidget *ui;
    bool m_dark = true;
};

#endif // NAVWIDGET_H
