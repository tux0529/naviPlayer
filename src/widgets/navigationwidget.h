#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>
#include <QListWidget>

class NavigationWidget : public QWidget
{
    Q_OBJECT

public:
    enum NavigationItem {
        Home,
        Favorites,
        Albums,
        Artists,
        Search,
        Playlist,
        Settings
    };

    explicit NavigationWidget(QWidget *parent = nullptr);
    void setCurrentItem(NavigationItem item);

signals:
    void itemSelected(NavigationItem item);

private slots:
    void onItemClicked(QListWidgetItem *item);

private:
    void setupUI();

    QListWidget *m_listWidget;
};

#endif // NAVIGATIONWIDGET_H
