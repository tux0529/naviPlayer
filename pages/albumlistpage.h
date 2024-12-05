#ifndef ALBUMLISTPAGE_H
#define ALBUMLISTPAGE_H

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

signals:
    void linkActivated(const QString &);
    void playIconClicked(const QString &);

private:
    void initForm();
    Ui::AlbumListPage *ui;

    AnchorTableView *m_listView;
    AlbumListModel *m_model;
    void resizeTableView();
};

#endif // ALBUMLISTPAGE_H
