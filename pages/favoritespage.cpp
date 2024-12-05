#include "favoritespage.h"
#include "ui_favoritespage.h"

#include "mediahelper.h"

#include "anchortableview.h"
#include "anchoritemdelegate.h"

#include "album.h"
#include "artist.h"
#include "track.h"


FavorTracksModel::FavorTracksModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int FavorTracksModel::rowCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent);
    return MediaHelper::s_favorTracks.size();
}

int FavorTracksModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant FavorTracksModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= MediaHelper::s_favorTracks.size() || index.row() < 0)
        return QVariant();

    const Track &trk = MediaHelper::s_favorTracks.at(index.row());

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
            return trk.icon();
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant FavorTracksModel::headerData(int section, Qt::Orientation orientation, int role) const
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





FavorAlbumsModel::FavorAlbumsModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int FavorAlbumsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return MediaHelper::s_favorAlbums.size();
}

int FavorAlbumsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant FavorAlbumsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= MediaHelper::s_favorAlbums.size() || index.row() < 0)
        return QVariant();

    const Album &album = MediaHelper::s_favorAlbums.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("<style>a{color: #CCCCCC; text-decoration:none;}p{margin: 4px 0px;}</style><p>%1</p> <p><a href='search://artist=%2'>%2</a></p>").arg(album.name(), album.artist());
        else if (index.column() == 2)
            return album.songCount();
        else if (index.column() == 3)
            return album.duration();
        break;
    case Qt::UserRole:
        return album.id();
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter ;
        break;
    case Qt::DecorationRole:
        if (index.column() == 1)
            return album.icon();
        break;
    default:
        break;
    }
    return QVariant();
}

QVariant FavorAlbumsModel::headerData(int section, Qt::Orientation orientation, int role) const
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

        default:
            return QVariant();
        }
    }
    return QVariant();
}




FavorArtistsModel::FavorArtistsModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int FavorArtistsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return MediaHelper::s_favorArtists.size();
}

int FavorArtistsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant FavorArtistsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= MediaHelper::s_favorArtists.size() || index.row() < 0)
        return QVariant();

    const Artist &art = MediaHelper::s_favorArtists.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  art.name();
        else if (index.column() == 2)
            return art.albumCount();
        break;
    case Qt::UserRole:
        return art.id();
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

QVariant FavorArtistsModel::headerData(int section, Qt::Orientation orientation, int role) const
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











FavoritesPage::FavoritesPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FavoritesPage)
{
    ui->setupUi(this);

    MediaHelper::Instance()->getFavorites();

    initForm();
}

FavoritesPage::~FavoritesPage()
{
    delete ui;
}

void FavoritesPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    int w = this->width();

    int col1 = 40;
    int sCol = w*0.1;
    //width of albums title column
    int trkCol = ( w - sCol -col1 - 60)/2;

    //width of albums title column
    int albCol = w - 2*sCol -col1 - 60;

    //width of artist title column
    int artCol = w - sCol -col1 - 60;

    m_tracksView->setColumnWidth(0, col1);
    m_tracksView->setColumnWidth(1, trkCol);
    m_tracksView->setColumnWidth(2, trkCol);
    m_tracksView->setColumnWidth(3, sCol);

    m_albumsView->setColumnWidth(0, col1);
    m_albumsView->setColumnWidth(1, albCol);
    m_albumsView->setColumnWidth(2, sCol);
    m_albumsView->setColumnWidth(3, sCol);

    m_artissView->setColumnWidth(0, col1);
    m_artissView->setColumnWidth(1, artCol);
    m_artissView->setColumnWidth(2, sCol);
}

void FavoritesPage::buttonClicked()
{
    QPushButton *b = qobject_cast<QPushButton *>(sender());
    QString name = b->objectName();

    ui->tracksButton->setChecked(false);
    ui->albumsButton->setChecked(false);
    ui->artistsButton->setChecked(false);

    if ( name == "tracksButton"){
        ui->tracksButton->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
    } else if ( name == "albumsButton"){
        ui->albumsButton->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
    } else if ( name == "artistsButton"){
        ui->artistsButton->setChecked(true);
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void FavoritesPage::initForm()
{

    ui->tracksButton->setCheckable(true);
    ui->albumsButton->setCheckable(true);
    ui->artistsButton->setCheckable(true);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);

    m_tracksView = new AnchorTableView(this);
    m_tracksModel = new FavorTracksModel(this);
    m_tracksView->setModel(m_tracksModel);
    m_tracksView->setItemDelegate(anchorItemDelegate);

    m_albumsView = new AnchorTableView(this);
    m_albumsModel = new FavorAlbumsModel(this);
    m_albumsView->setModel(m_albumsModel);
    m_albumsView->setItemDelegate(anchorItemDelegate);

    m_artissView = new AnchorTableView(this);
    m_artistsModel = new FavorArtistsModel(this);
    m_artissView->setModel(m_artistsModel);
    m_artissView->setItemDelegate(anchorItemDelegate);

    ui->stackedWidget->insertWidget(0, m_tracksView);
    ui->stackedWidget->insertWidget(1, m_albumsView);
    ui->stackedWidget->insertWidget(2, m_artissView);

    connect(ui->tracksButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->albumsButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(ui->artistsButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    ui->tracksButton->click();
}
