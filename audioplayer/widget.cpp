#include "widget.h"
#include "ui_widget.h"
#include "playbackqueue.h"
#include "audioplayer.h"

#include "../common/track.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setAcceptDrops(true);

    ui->tableView->setShowGrid(false);

    ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(40);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->verticalHeader()->setDefaultSectionSize(56);
    ui->tableView->setAlternatingRowColors(false);

    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    m_player = new AudioPlayer(this);
    m_playbackQueue = new PlaybackQueue(this);

    ui->tableView->setModel(m_playbackQueue);
    m_player->setPlaybackQueue(m_playbackQueue);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())      //判断拖的类型
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())        //判断放的类型
    {

        QList<QUrl> List = event->mimeData()->urls();

        for(int i = 0; i < List.size(); ++i){
            QFileInfo fi(List.at(i).toLocalFile());
            if (fi.isFile()){
                Track trk;
                trk.setCacheFilepath(fi.canonicalFilePath());
                trk.setName(fi.fileName());
                m_playbackQueue->addTrack(trk);
            }
            else if (fi.isDir()){
                QDir dir(List.at(i).toLocalFile());
                QStringList filters;
                filters << "*.mp3" << "*.m4a" << "*.flac" << "*.wav" << "*.wma" << "*.wv" << "*.opus" << "*.ogg";
                QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
                foreach (auto fileInfo, fileInfoList) {
                    Track trk;
                    trk.setCacheFilepath(fileInfo.canonicalFilePath());
                    trk.setName(fileInfo.fileName());
                    m_playbackQueue->addTrack(trk);
                }
            }
        }

    }
    else
    {
        event->ignore();
    }
}


void Widget::on_pushButton_clicked()
{

}

