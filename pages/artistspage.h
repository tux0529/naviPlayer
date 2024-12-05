#ifndef ARTISTSPAGE_H
#define ARTISTSPAGE_H



#include <QAbstractTableModel>

class ArtistsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ArtistsModel(QObject *parent = nullptr);
    ~ArtistsModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //Qt::ItemFlags flags(const QModelIndex &index) const override;
    //bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
};







#include <QWidget>

namespace Ui {
class ArtistsPage;
}

class ArtistsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ArtistsPage(QWidget *parent = nullptr);
    ~ArtistsPage();

protected:
    void resizeEvent(QResizeEvent *event);
private:
    Ui::ArtistsPage *ui;
    void initForm();
};

#endif // ARTISTSPAGE_H
