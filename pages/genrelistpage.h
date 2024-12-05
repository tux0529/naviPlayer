#ifndef GENRELISTPAGE_H
#define GENRELISTPAGE_H

#include <QWidget>


#include <QAbstractTableModel>

class GenreListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    GenreListModel(QObject *parent = nullptr);
    ~GenreListModel();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //Qt::ItemFlags flags(const QModelIndex &index) const override;
    //bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
};



namespace Ui {
class GenreListPage;
}

class GenreListPage : public QWidget
{
    Q_OBJECT

public:
    explicit GenreListPage(QWidget *parent = nullptr);
    ~GenreListPage();
protected:
    void resizeEvent(QResizeEvent *event);

private:
    void initForm();
    Ui::GenreListPage *ui;
};

#endif // GENRELISTPAGE_H
