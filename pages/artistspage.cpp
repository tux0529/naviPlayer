#include "artistspage.h"
#include "ui_artistspage.h"
#include "mediahelper.h"
#include "anchoritemdelegate.h"


ArtistsModel::ArtistsModel(QObject *parent) : QAbstractTableModel(parent)
{

}

ArtistsModel::~ArtistsModel()
{

}
int ArtistsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return MediaHelper::s_artistList.size();
}

int ArtistsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ArtistsModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= MediaHelper::s_artistList.size() || index.row() < 0)
        return QVariant();

    const Artist &art = MediaHelper::s_artistList.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  art.name();
        else if (index.column() == 2)
            return art.albumCount();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return art.icon();
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant ArtistsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Albums");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}




ArtistsPage::ArtistsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ArtistsPage)
{
    ui->setupUi(this);
    MediaHelper::Instance()->getArtists();
    initForm();
}

ArtistsPage::~ArtistsPage()
{
    delete ui;
}

void ArtistsPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    int w = this->width();
    int col1 = 40;
    int col3 = w*0.1;
    int col2 = w - col3 -col1 - 60;

    ui->tableView->setColumnWidth(0, col1);
    ui->tableView->setColumnWidth(1, col2);
    ui->tableView->setColumnWidth(2, col3);
}

void ArtistsPage::initForm()
{
    ArtistsModel *model = new ArtistsModel(this);

    ui->tableView->setModel(model);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);
    ui->tableView->setItemDelegate(anchorItemDelegate);
}
