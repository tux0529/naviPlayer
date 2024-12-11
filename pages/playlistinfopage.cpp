#include "playlistinfopage.h"
#include "ui_playlistinfopage.h"
#include "anchoritemdelegate.h"

#include "mediahelper.h"



PlaylistInfoModel::PlaylistInfoModel(QObject *parent):
    QAbstractTableModel(parent),
    m_playlistInfoPage(dynamic_cast<PlaylistInfoPage*>(parent))
{

}

PlaylistInfoModel::~PlaylistInfoModel()
{

}

void PlaylistInfoModel::updateData(const QString &id)
{
    beginResetModel();
    m_playlistInfoPage->m_playlistObject = MediaHelper::Instance()->getPlaylist(id);
    endResetModel();
}

int PlaylistInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_playlistInfoPage->m_playlistObject.songCount();
}

int PlaylistInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant PlaylistInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_playlistInfoPage->m_playlistObject.songCount() || index.row() < 0)
        return QVariant();

    const Track &trk = m_playlistInfoPage->m_playlistObject.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("<style>a{color: #CCCCCC; text-decoration:none;}p{margin: 4px 0px;}</style><p>%1</p> <p><a href='search://artist=%2'>%2</a></p>").arg(trk.name(), trk.artist());
        else if (index.column() == 2)
            return trk.album();
        else if (index.column() == 3)
            return trk.duration();
        break;
    case Qt::UserRole:
        return trk.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return MediaHelper::getAlbumIcon(trk.albumId());
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant PlaylistInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Album");
            break;

        case 3:
            return tr("Duration");
            break;

        default:
            return QVariant();
        }
    }
    return QVariant();
}










PlaylistInfoPage::PlaylistInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlaylistInfoPage)
{
    ui->setupUi(this);

    m_playlistInfoModel = new PlaylistInfoModel(this);
    ui->tableView->setModel(m_playlistInfoModel);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);
    ui->tableView->setItemDelegate(anchorItemDelegate);
}

PlaylistInfoPage::~PlaylistInfoPage()
{
    delete ui;
}

void PlaylistInfoPage::loadPlaylist(const QString &id)
{
    m_playlistInfoModel->updateData(id);
    ui->posterLabel->setPixmap(QPixmap(MediaHelper::Instance()->getCoverArt(m_playlistObject.id(), MediaHelper::PlayListCover, MediaHelper::Poster)));
    ui->pageTitleLabel->setText(m_playlistObject.name());
    ui->pageInfoLabel->setText(tr("Playlist by %3 · %1 songs, %2 ")
                                   .arg( m_playlistObject.songCount() )
                                   .arg( m_playlistObject.duration() )
                                   .arg( m_playlistObject.owner() ) );
}
