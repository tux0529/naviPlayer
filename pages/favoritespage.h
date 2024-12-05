#ifndef FAVORITESPAGE_H
#define FAVORITESPAGE_H


#include <QAbstractTableModel>

class FavorTracksModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FavorTracksModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

};


class FavorAlbumsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FavorAlbumsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

};


class FavorArtistsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FavorArtistsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

};






#include <QWidget>

namespace Ui {
class FavoritesPage;
}

class AnchorTableView;

class FavoritesPage : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritesPage(QWidget *parent = nullptr);
    ~FavoritesPage();

protected:
    void resizeEvent(QResizeEvent *event);
private slots:
    void buttonClicked();

private:

    Ui::FavoritesPage *ui;

    AnchorTableView *m_tracksView;
    AnchorTableView *m_albumsView;
    AnchorTableView *m_artissView;

    FavorTracksModel *m_tracksModel;
    FavorAlbumsModel *m_albumsModel;
    FavorArtistsModel *m_artistsModel;


    void initForm();
};

#endif // FAVORITESPAGE_H

