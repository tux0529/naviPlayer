#ifndef ALBUMINFOPAGE_H
#define ALBUMINFOPAGE_H

#include "albumobject.h"

#include <QWidget>

#include <QAbstractTableModel>

class AlbumInfoPage;

class AlbumInfoModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AlbumInfoModel(QObject *parent = nullptr);
    ~AlbumInfoModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    void updateAlbum(const QString &albumId);

private:
    AlbumInfoPage *m_albumInfoPage;
};






namespace Ui {
class albumInfoPage;
}

class AnchorItemDelegate;

class AlbumInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumInfoPage(QWidget *parent = nullptr);
    ~AlbumInfoPage();

    void loadAlbum(const QString &albumId);

signals:
    void linkActivated(const QString &);
//     void rowDoubleClicked(Track tk);

// private slots:
//     void onRowDoubleClicked(const QModelIndex &index);
private:
    Ui::albumInfoPage *ui;

    AnchorItemDelegate *m_anchorItemDelegate;
    AlbumInfoModel *m_albumInfoModel;

    AlbumObject m_albumObject;

    void initForm();

    friend AlbumInfoModel;

};

#endif // ALBUMINFOPAGE_H
