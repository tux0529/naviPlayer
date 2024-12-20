#include "cardwidget.h"
#include "ui_cardwidget.h"
#include <QPixmapCache>

#include "floatframe.h"
#include "mediahelper.h"

const int CardWidget::width = 180;

CardWidget::CardWidget(const Album &album,
                       QWidget *parent) :
      QWidget(parent)
    , ui(new Ui::CardWidget)
    , m_album(album)
{
    ui->setupUi(this);
    this->resize(CardWidget::width, CardWidget::width + 36);
    ui->icoLabel->resize(CardWidget::width, CardWidget::width);
    ui->icoLabel->setAlignment(Qt::AlignCenter);

    QImage img(MediaHelper::Instance()->getCoverArt(m_album.id(), MediaHelper::AlbumCover, MediaHelper::Poster ));
    ui->icoLabel->setPixmap(QPixmap::fromImage( img));
    ui->icoLabel->installEventFilter(this);

    ui->titleLabel->setText(QString("<a style='color: #DCDCDC; text-decoration:none;' href='search://album=%1'>%2</a>").arg(m_album.id(), m_album.name()) );

    if (m_album.artist().isEmpty())
        ui->artistLabel->setVisible(false);
    else {
        ui->artistLabel->setVisible(true);
        ui->artistLabel->setText(QString("<a style='color: #DCDCDC; text-decoration:none;' href='search://artist=%1'>%2</a>").arg(m_album.artistId(), m_album.artist()));
    }

    m_playFrame = new FloatFrame(this);
    m_playFrame->setFillBackground(false);
    m_playFrame->setHideInterval(10);

    connect(ui->titleLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(ui->artistLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
    connect(m_playFrame, SIGNAL(mouseClicked()), this, SLOT(iconClicked()));

    QPixmap maskPixmap;
    if (!QPixmapCache::find("playMask", &maskPixmap)) {
        maskPixmap.load(":/images/playMask.png");
        QPixmapCache::insert("playMask", maskPixmap);
    }

    m_label = new QLabel(m_playFrame);

    m_label->resize(maskPixmap.width(), maskPixmap.height());
    m_label->setPixmap(maskPixmap);
    m_label->setAutoFillBackground(false);
    m_label->setAttribute(Qt::WA_TranslucentBackground);
    m_playFrame->addWidget(m_label);

    QHBoxLayout *hlayout = new QHBoxLayout(m_playFrame);
    hlayout->setObjectName(QString::fromUtf8("hlayout"));
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addWidget(m_label);
    m_playFrame->setLayout(hlayout);

    m_playFrame->resize(CardWidget::width, CardWidget::width);
    m_playFrame->move(0,0);

    m_playFrame->installEventFilter(this);
}

CardWidget::~CardWidget()
{
    delete ui;
}

bool CardWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_playFrame){
        switch (event->type()) {
        case QEvent::Leave:
            setCursor(Qt::ArrowCursor);
            //qDebug("Mouse Leave");
            break;
        case QEvent::Enter:
            setCursor(Qt::PointingHandCursor);
            //qDebug("Mouse Enter");
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(obj, event);
}

void CardWidget::iconClicked()
{
    emit playIconClicked(m_album.id());
}



