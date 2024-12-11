#include "playlistspage.h"
#include "ui_playlistspage.h"
#include "iconhelper.h"
#include "mediahelper.h"
#include "anchoritemdelegate.h"
#include "anchortableview.h"



PlaylistsModel::PlaylistsModel(QObject *parent) : QAbstractTableModel(parent)
{
    updateData();
}

int PlaylistsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_playLists.size();
}

int PlaylistsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant PlaylistsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_playLists.size() || index.row() < 0)
        return QVariant();

    const PlayList &pl = m_playLists.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return pl.name();
        else if (index.column() == 2)
            return pl.songCount();
        else if (index.column() == 3)
            return pl.duration();
        else if (index.column() == 4)
            return pl.duration();
        break;
    case Qt::UserRole:
        return pl.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return pl.icon();
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant PlaylistsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Duration");
            break;

        case 4:
            return tr("Owner");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}

void PlaylistsModel::updateData()
{
    beginResetModel();
    m_playLists.clear();
    m_playLists = MediaHelper::Instance()->getPlaylists();
    endResetModel();
}







PlaylistsPage::PlaylistsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaylistsPage)
{
    ui->setupUi(this);
    initForm();
}

PlaylistsPage::~PlaylistsPage()
{
    delete ui;
}

void PlaylistsPage::onRowDoubleClicked(const QModelIndex &index)
{
    emit rowDoubleClicked(m_playlistsModel->data(index, Qt::UserRole).toString());
}

void PlaylistsPage::initForm()
{

    IconHelper::Instance()->setIcon(ui->refreshButton, QChar(0xf021));

    m_playlistsModel = new PlaylistsModel(this);

    m_anchorItemDelegate = new AnchorItemDelegate(this);
    ui->tableView->setItemDelegate(m_anchorItemDelegate);

    ui->tableView->setModel(m_playlistsModel);

    connect(ui->tableView, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));
}

void PlaylistsPage::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event)
    resizeTableView();
}

void PlaylistsPage::resizeTableView()
{
    int w = this->width();
    int col1 = 40;
    int col3 = w*0.15;
    int col4 = w*0.15;
    int col5 = w*0.2;
    int col2 = w - col3 -col4 - col5 -col1 -20;

    ui->tableView->setColumnWidth(0, col1);
    ui->tableView->setColumnWidth(1, col2);
    ui->tableView->setColumnWidth(2, col3);
    ui->tableView->setColumnWidth(3, col4);
    ui->tableView->setColumnWidth(4, col5);
}
