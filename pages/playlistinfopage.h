#ifndef PLAYLISTINFOPAGE_H
#define PLAYLISTINFOPAGE_H

#include "playlistobject.h"

#include <QWidget>

#include <QAbstractTableModel>

class PlaylistInfoPage;

class PlaylistInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PlaylistInfoModel(QObject *parent = nullptr);
    ~PlaylistInfoModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    void updateData(const QString &id);

private:
    PlaylistInfoPage *m_playlistInfoPage;
};


















namespace Ui {
class PlaylistInfoPage;
}

class PlaylistInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistInfoPage(QWidget *parent = nullptr);
    ~PlaylistInfoPage();

    void loadPlaylist(const QString &id);

private:
    Ui::PlaylistInfoPage *ui;

    PlaylistInfoModel *m_playlistInfoModel;

    PlayListObject m_playlistObject;


    friend PlaylistInfoModel;
};

#endif // PLAYLISTINFOPAGE_H
