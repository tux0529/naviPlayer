#ifndef PLAYLISTSPAGE_H
#define PLAYLISTSPAGE_H

#include "playlist.h"

#include <QAbstractTableModel>

class PlaylistsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    PlaylistsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void updateData();

private:
    QList<PlayList> m_playLists;
};






#include <QWidget>

namespace Ui {
class PlaylistsPage;
}

class  AnchorItemDelegate;

class PlaylistsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistsPage(QWidget *parent = nullptr);
    ~PlaylistsPage();


signals:
    void linkActivated(const QString &);
    void rowDoubleClicked();

protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void onRowDoubleClicked(const QModelIndex &index);
private:
    Ui::PlaylistsPage *ui;
    AnchorItemDelegate *m_anchorItemDelegate;
    PlaylistsModel *m_playlistsModel;

    void initForm();
    void resizeTableView();
};

#endif // PLAYLISTSPAGE_H
