#include "albuminfopage.h"
#include "ui_albuminfopage.h"
#include "anchoritemdelegate.h"
#include "application.h"

#include "mediahelper.h"
#include "iconhelper.h"



AlbumInfoModel::AlbumInfoModel(QObject *parent):
    QAbstractTableModel(parent),
    m_albumInfoPage(dynamic_cast<AlbumInfoPage*>(parent))
{

}

AlbumInfoModel::~AlbumInfoModel()
{

}

void AlbumInfoModel::updateAlbum(const QString &albumId)
{
    beginResetModel();
    m_albumInfoPage->m_albumObject = MediaHelper::Instance()->getAlbum(albumId);
    endResetModel();
}

int AlbumInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_albumInfoPage->m_albumObject.songCount();
}

int AlbumInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant AlbumInfoModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_albumInfoPage->m_albumObject.songCount() || index.row() < 0)
        return QVariant();

    const Track &trk = m_albumInfoPage->m_albumObject.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0)
            return  index.row() + 1;
        else if (index.column() == 1)
            return  QString("<style>a{color: #CCCCCC; text-decoration:none;}p{margin: 4px 0px;}</style><p>%1</p> <p><a href='search://artist=%2'>%2</a></p>").arg(trk.name(), trk.artist());
        else if (index.column() == 2)
            return trk.artist();
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

QVariant AlbumInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            return tr("Artist");
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






AlbumInfoPage::AlbumInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::albumInfoPage)
{
    ui->setupUi(this);
    initForm();
}

AlbumInfoPage::~AlbumInfoPage()
{
    delete ui;
}

void AlbumInfoPage::loadAlbum(const QString &albumId)
{
    m_albumInfoModel->updateAlbum(albumId);
    ui->posterLabel->setPixmap(QPixmap(MediaHelper::Instance()->getCoverArt(m_albumObject.id(), MediaHelper::AlbumCover, MediaHelper::Poster)));
    ui->pageTitleLabel->setText(m_albumObject.name());
    ui->pageInfoLabel->setText(tr("Album by %3 · %1 songs, %2 ")
                                   .arg(m_albumObject.songCount() )
                                   .arg( m_albumObject.duration() )
                                   .arg( m_albumObject.artist() ));
}

void AlbumInfoPage::initForm()
{
    m_albumInfoModel = new AlbumInfoModel(this);
    ui->tableView->setModel(m_albumInfoModel);

    m_anchorItemDelegate = new AnchorItemDelegate(this);

    ui->tableView->setItemDelegate(m_anchorItemDelegate);

    IconHelper::Instance()->setIcon(ui->playButton, QChar(0xF144), 36);
    IconHelper::Instance()->setIcon(ui->appendToQueueEndButton, QChar(0xF067), 24);
    IconHelper::Instance()->setIcon(ui->insertToQueueButton, QChar(0xF055), 24);
    IconHelper::Instance()->setIcon(ui->favorButton, QChar(0xF08a), 24);

    connect(ui->playButton, SIGNAL(clicked()),
            this, SIGNAL(linkActivated(const QString &)));
    connect(ui->appendToQueueEndButton, SIGNAL(clicked()),
            this, SIGNAL(linkActivated(const QString &)));
    connect(ui->insertToQueueButton, SIGNAL(clicked()),
            this, SIGNAL(linkActivated(const QString &)));
    connect(ui->favorButton, SIGNAL(clicked()),
            this, SIGNAL(linkActivated(const QString &)));

    connect(ui->tableView, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    //connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));
}

