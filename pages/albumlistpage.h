#ifndef ALBUMLISTPAGE_H
#define ALBUMLISTPAGE_H

#include "qmenu.h"
#include <QWidget>

namespace Ui {
class AlbumListPage;
}
class AnchorTableView;
class AlbumListModel;

class AlbumListPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumListPage(QWidget *parent = nullptr);
    ~AlbumListPage();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void onRowDoubleClicked(const QModelIndex &index);
    void updateData(const QString &page = QString("1"));
    void on_comboBox_currentIndexChanged(int index);
    void onContextMenuRequested(const QPoint &pos);

    void onPlayAlbum();
    void onInsertToQueue();
    void onAppendToQueue();

signals:
    void linkActivated(const QString &);
    void playIconClicked(const QString &);

private:
    void initForm();

    void resizeTableView();

    QString pageString(int page);

    Ui::AlbumListPage *ui;

    AnchorTableView *m_listView;
    AlbumListModel *m_model;

    QMenu *m_contextMenu;
    QAction *m_playAction;
    QAction *m_insertToQueueAction;
    QAction *m_appendToQueueAction;
    QAction *m_addToPlaylistAction;
    QAction *m_addToFavoriteAction;
    QAction *m_removeFromFavoriteAction;
};

#endif // ALBUMLISTPAGE_H
