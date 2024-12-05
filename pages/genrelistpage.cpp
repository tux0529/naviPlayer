#include "genrelistpage.h"
#include "ui_genrelistpage.h"
#include "mediahelper.h"
#include "anchoritemdelegate.h"


GenreListModel::GenreListModel(QObject *parent) : QAbstractTableModel(parent)
{

}

GenreListModel::~GenreListModel()
{

}


int GenreListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return MediaHelper::s_genreList.size();
}

int GenreListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant GenreListModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= MediaHelper::s_genreList.size() || index.row() < 0)
        return QVariant();

    const Genre gr = MediaHelper::s_genreList.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  gr.name();
        else if (index.column() == 2)
            return gr.songCount();
        else if (index.column() == 3)
            return gr.albumCount();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant GenreListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("#");
            break;

        case 1:
            return tr("Title");
            break;

        case 2:
            return tr("Tracks");
            break;

        case 3:
            return tr("Albums");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}




GenreListPage::GenreListPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenreListPage)
{
    ui->setupUi(this);
    MediaHelper::Instance()->getGenres();
    initForm();
}

GenreListPage::~GenreListPage()
{
    delete ui;
}

void GenreListPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    int w = this->width();
    int col1 = 40;
    int col3 = w*0.1;
    int col4 = w*0.1;
    int col2 = w - col3 -col4 -col1 - 60;

    ui->tableView->setColumnWidth(0, col1);
    ui->tableView->setColumnWidth(1, col2);
    ui->tableView->setColumnWidth(2, col3);
    ui->tableView->setColumnWidth(3, col4);
}

void GenreListPage::initForm()
{
    GenreListModel *model = new GenreListModel(this);

    ui->tableView->setModel(model);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);
    ui->tableView->setItemDelegate(anchorItemDelegate);
}
