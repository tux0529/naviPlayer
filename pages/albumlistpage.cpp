#include "albumlistpage.h"
#include "qlistview.h"
#include "ui_albumlistpage.h"
#include "anchortableview.h"
#include "anchoritemdelegate.h"
#include "albumlistmodel.h"
#include "config.h"


AlbumListPage::AlbumListPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlbumListPage)
{
    ui->setupUi(this);
    initForm();
    updateData();
}

AlbumListPage::~AlbumListPage()
{
    delete ui;
}

void AlbumListPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    resizeTableView();
}

void AlbumListPage::resizeTableView()
{
    int w = this->width();
    int col1 = 40;
    int col3 = w*0.1;
    int col4 = w*0.1;
    int col2 = w - col3 -col4 -col1 - 60;

    m_listView->setColumnWidth(0, col1);
    m_listView->setColumnWidth(1, col2);
    m_listView->setColumnWidth(2, col3);
    m_listView->setColumnWidth(3, col4);
}

QString AlbumListPage::pageString(int page)
{
    QString pagerString = "<style>a{color: #DCDCDC; text-decoration:none;white-space: pre;} ";
    pagerString += "span{color: #00BB9E;}</style>";

    if(MediaHelper::s_albumPageSize < 5){
        for(int i = 0; i < MediaHelper::s_albumPageSize; ++i){

            if (page == i +1){
                pagerString += QString("<span> %1 </span>").arg(i+1);
            }
            else{
                pagerString += QString("<a href='%1'> %1 </a>").arg(i+1);
            }
        }
    }
    else{

        if(page > 1){
            pagerString += QString("<a href='%1'> ＜ </a>").arg(page - 1);
            pagerString += QString("<a href='1'> 1 </a>");
        }
        else{
            pagerString += QString("<span> ＜ </span>");
            pagerString += QString("<span> 1 </span>");
        }

        if(page > 3){
            pagerString += QString("<span> … </span>");
        }

        if (page == 1){
            pagerString += QString("<a href='2'> 2 </a>");
            pagerString += QString("<a href='3'> 3 </a>");
        }
        else if (page == 2){
            pagerString += QString("<span> 2 </span>");
            pagerString += QString("<a href='3'> 3 </a>");
        }
        else if(page == MediaHelper::s_albumPageSize - 1){
            pagerString += QString("<a href='%1'> %1 </a>").arg(page-1);
            pagerString += QString("<span> %1 </span>").arg(page);
        }
        else if(page == MediaHelper::s_albumPageSize){
            pagerString += QString("<a href='%1'> %1 </a>").arg(page-1);
            pagerString += QString("<a href='%1'> %1 </a>").arg(page);
        }
        else{
            pagerString += QString("<a href='%1'> %1 </a>").arg(page-1);
            pagerString += QString("<span> %1 </span>").arg(page);
            pagerString += QString("<a href='%1'> %1 </a>").arg(page+1);
        }

        if(MediaHelper::s_albumPageSize - page > 2){
            pagerString += QString("<span> … </span>");
        }

        if(page < MediaHelper::s_albumPageSize){
            pagerString += QString("<a href='%1'> %1 </a>").arg(MediaHelper::s_albumPageSize);
            pagerString += QString("<a href='%1'> ＞ </a>").arg(page + 1);
        }
        else{
            pagerString += QString("<span> %1 </span>").arg(MediaHelper::s_albumPageSize);
            pagerString += QString("<span> ＞ </span>");
        }
    }
    Config::G_Debug("AlbumListPage::pagerString: ", pagerString);

    return pagerString;

}

void AlbumListPage::onRowDoubleClicked(const QModelIndex &index)
{

    if (!index.isValid()){
        return;
    }

    emit linkActivated(QString("search://album=%1").arg(m_model->data(index, Qt::UserRole).toString()));

    //Config::G_Debug("AlbumListPage::onRowDoubleClicked:Row: ", index.row());
    //Config::G_Debug("AlbumListPage::onRowDoubleClicked:albumId: ", m_model->data(index, Qt::UserRole).toString());
}

void AlbumListPage::updateData(const QString &page)
{
    Config::G_Debug("AlbumListPage::updateData:Page clicked: ", page);

    Config::G_Debug("AlbumListPage::updateData:albumPageSize: ", MediaHelper::s_albumPageSize);

    int p = page.toInt();

    int albumEndNum;
    if (p > MediaHelper::s_albumPageSize){
        return;
    }
    else if (p == MediaHelper::s_albumPageSize){
        albumEndNum = MediaHelper::s_albumCount;
    }
    else{
        albumEndNum = p * Config::s_options.albumCountPerPage;
    }

    m_model->updateData((MediaHelper::AlbumType)ui->comboBox->currentIndex(), Config::s_options.albumCountPerPage,
                        (p-1)*Config::s_options.albumCountPerPage);

    ui->label->setText(QString("%1 - %2").arg((p-1)*Config::s_options.albumCountPerPage + 1).arg(albumEndNum));

    /*
    QString pagerString = "<style>a{color: #DCDCDC; text-decoration:none;white-space: pre;} ";
    pagerString += "span{color: #00BB9E;}</style>";

    for(int i = 0; i < MediaHelper::s_albumPageSize; ++i){

        if (p == i +1){
            pagerString += QString("<span> %1 </span>").arg(i+1);
        }
        else{
            pagerString += QString("<a href='%1'> %1 </a>").arg(i+1);
        }
    }
    */

    ui->pagerLabel->setText( this->pageString(p) );

    m_listView->scrollToTop();
}

void AlbumListPage::initForm()
{
    m_listView = new AnchorTableView(this);

    m_model = new AlbumListModel(this);

    m_listView->setModel(m_model);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);
    m_listView->setItemDelegate(anchorItemDelegate);

    m_listView->setContextMenuPolicy( Qt::CustomContextMenu );
    m_contextMenu = new QMenu(m_listView);
    m_contextMenu->setWindowFlags( m_contextMenu->windowFlags() | Qt::FramelessWindowHint);
    m_contextMenu->setAttribute(Qt::WA_TranslucentBackground);

    m_playAction = new QAction(tr("Play"), m_listView);
    m_insertToQueueAction = new QAction(tr("Insert to queue"), m_listView);
    m_appendToQueueAction = new QAction(tr("Append to queue"), m_listView);
    m_addToPlaylistAction = new QAction(tr("Add to Playlist"), m_listView);
    m_addToFavoriteAction = new QAction(tr("Add to Favorites"), m_listView);
    m_removeFromFavoriteAction = new QAction(tr("Remove from Favorites"), m_listView);

    m_contextMenu->addAction(m_playAction);
    m_contextMenu->addAction(m_insertToQueueAction);
    m_contextMenu->addAction(m_appendToQueueAction);
    m_contextMenu->addAction(m_addToPlaylistAction);

    m_contextMenu->addSeparator();

    m_contextMenu->addAction(m_addToFavoriteAction);
    m_contextMenu->addAction(m_removeFromFavoriteAction);

    QListView *listView = new QListView(ui->comboBox);
    listView->setWindowFlags( listView->windowFlags() | Qt::FramelessWindowHint);
    listView->setAttribute(Qt::WA_TranslucentBackground);
    ui->comboBox->setView(listView);
    ui->comboBox->addItem(QString());
    ui->comboBox->addItem(QString());
    ui->comboBox->addItem(QString());
    ui->comboBox->addItem(QString());
    ui->comboBox->setItemText(0, QCoreApplication::translate("AlbumListPage", "Recent Added", nullptr));
    ui->comboBox->setItemText(1, QCoreApplication::translate("AlbumListPage", "Most Played", nullptr));
    ui->comboBox->setItemText(2, QCoreApplication::translate("AlbumListPage", "Recent Played", nullptr));
    ui->comboBox->setItemText(3, QCoreApplication::translate("AlbumListPage", "Random", nullptr));

    ui->stackedWidget->insertWidget(0, m_listView);
    ui->stackedWidget->setCurrentIndex(0);

    connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onContextMenuRequested(const QPoint &) ) );
    connect(m_playAction, SIGNAL(triggered()), this, SLOT(onPlayAlbum() ) );
    connect(m_insertToQueueAction, SIGNAL(triggered()), this, SLOT(onInsertToQueue() ) );
    connect(m_appendToQueueAction, SIGNAL(triggered()), this, SLOT(onAppendToQueue() ) );

    connect(m_listView, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(m_listView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));
    connect(ui->pagerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(updateData(const QString &)));

}

void AlbumListPage::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    this->updateData();
}

void AlbumListPage::onContextMenuRequested(const QPoint &pos)
{

    QModelIndex index = m_listView->indexAt(pos);
    if(index.isValid()){
        //QModelIndexList mIndexList = m_listView->selectionModel()->selectedRows();
        m_contextMenu->exec(QCursor::pos());
    }
}

void AlbumListPage::onPlayAlbum()
{
    QModelIndexList mIndexList = m_listView->selectionModel()->selectedRows();
    for(int i = mIndexList.size(); i > 0; --i){
        Config::G_Debug("AlbumListPage::onPlayAlbum:row: ", mIndexList.at(i-1).row());
        emit playIconClicked(m_model->data(mIndexList.at(i-1), Qt::UserRole).toString());
    }
}

void AlbumListPage::onInsertToQueue()
{

}

void AlbumListPage::onAppendToQueue()
{
    QModelIndexList mIndexList = m_listView->selectionModel()->selectedRows();
    for(int i = mIndexList.size(); i > 0; --i){
        emit playIconClicked(m_model->data(mIndexList.at(i-1), Qt::UserRole).toString());
    }
}

