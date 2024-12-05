#include "albumlistpage.h"
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

void AlbumListPage::onRowDoubleClicked(const QModelIndex &index)
{

    if (!index.isValid()){
        return;
    }

    Config::G_Debug("AlbumListPage::onRowDoubleClicked:Row: ", index.row());
    Config::G_Debug("AlbumListPage::onRowDoubleClicked:albumId: ", m_model->data(index, Qt::UserRole).toString());

    emit playIconClicked(m_model->data(index, Qt::UserRole).toString());
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

    Config::G_Debug("pagerString: ", pagerString);

    ui->pagerLabel->setText( pagerString );

    m_listView->scrollToTop();
}

void AlbumListPage::initForm()
{
    m_listView = new AnchorTableView(this);

    m_model = new AlbumListModel(this);

    m_listView->setModel(m_model);

    AnchorItemDelegate *anchorItemDelegate = new AnchorItemDelegate(this);
    m_listView->setItemDelegate(anchorItemDelegate);

    ui->stackedWidget->insertWidget(0, m_listView);
    ui->stackedWidget->setCurrentIndex(0);

    connect(m_listView, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(m_listView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));
    connect(ui->pagerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(updateData(const QString &)));

}

void AlbumListPage::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    this->updateData();
}

