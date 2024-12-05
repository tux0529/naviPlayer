#include "playqueuewidget.h"
#include "ui_playqueuewidget.h"
#include "iconhelper.h"
#include "anchortableview.h"
#include "anchoritemdelegate.h"

#include "playbackqueue.h"

//#include <QListView>


#include <QDebug>

#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
#include <QGraphicsDropShadowEffect>
#endif

PlayQueueWidget::PlayQueueWidget(PlaybackQueue *playQueue, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayQueueWidget)
    , m_playbackQueue (playQueue)
    , m_anchorItemDelegate (new AnchorItemDelegate(this))
{
    ui->setupUi(this);
    initForm();
}

PlayQueueWidget::~PlayQueueWidget()
{
    delete ui;
}

void PlayQueueWidget::resizeEvent(QResizeEvent *event){
    Q_UNUSED(event)
    resizeTableView();
}

// If you subclass from QWidget, you need to provide a paintEvent for your custom QWidget as below:
void PlayQueueWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);//读取qss设置的样式
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void PlayQueueWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (  !ui->tableView->geometry().contains(event->pos()) ){
        m_anchorItemDelegate->setHoverRow(-1);
    }
    qDebug() << ui->tableView->geometry();
    qDebug() << event->pos();
    return;
}

void PlayQueueWidget::onTrackCountChanged(int count)
{
    ui->infoLabel->setText(tr("%1 songs").arg(count));
}

// void PlayQueueWidget::onCurrentIndexChanged(int index)
// {
//     update();
//     emit currentSongChanged(s);
// }

void PlayQueueWidget::onRowDoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
        m_playbackQueue->setCurrentIndex(index.row());

}

void PlayQueueWidget::setPlaylist(PlaybackQueue *playQueue)
{
    this->m_playbackQueue = playQueue;
}

void PlayQueueWidget::initForm()
{
    //this->setWindowFlags(Qt::FramelessWindowHint);

     //setAttribute(Qt::WA_TranslucentBackground);
 #if (QT_VERSION >= QT_VERSION_CHECK(4,6,0))
     QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
     shadow->setOffset(0,0);
     shadow->setColor(QColor(0,0,0));
     shadow->setBlurRadius(9);
     this->setGraphicsEffect(shadow);
 #endif

    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    foreach (QWidget *w, widgets) {
        w->setAttribute(Qt::WA_TranslucentBackground);
    }

    IconHelper::Instance()->setIcon(ui->clearButton, QChar(0xF048), 16);
    IconHelper::Instance()->setIcon(ui->playbackModeButton, QChar(0xF144), 16);
    IconHelper::Instance()->setIcon(ui->moveUpButton, QChar(0xF051), 16);
    IconHelper::Instance()->setIcon(ui->moveDownButton, QChar(0xF08a), 16);
    IconHelper::Instance()->setIcon(ui->removeButton, QChar(0xF028), 16);

    ui->tableView->setModel(m_playbackQueue);

    ui->tableView->setItemDelegate(m_anchorItemDelegate);

    connect(m_playbackQueue, SIGNAL(trackCountChanged(int)), this, SLOT(onTrackCountChanged(int)));

    connect(ui->tableView, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));

    //connect(m_playbackQueue, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentIndexChanged(int)));
}

void PlayQueueWidget::resizeTableView()
{
    int w = ui->tableView->width();
    int col1 = 40;
    int col3 = 60;
    int col4 = 60;
    int col2 = w - col3 -col4 -col1 -15;

    ui->tableView->setColumnWidth(0, col1);
    ui->tableView->setColumnWidth(1, col2);
    ui->tableView->setColumnWidth(2, col3);
    ui->tableView->setColumnWidth(3, col4);
}
