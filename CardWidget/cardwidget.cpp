#include "cardwidget.h"
#include "ui_cardwidget.h"
#include <QDebug>

#include "floatframe.h"

const int CardWidget::width = 180;

CardWidget::CardWidget(const QString &albumId,
                       const QPixmap &icoPixmap,
                       const QString &title,
                       const QString &artistName,
                       QWidget *parent) :
      QWidget(parent)
    , ui(new Ui::CardWidget)
    , m_albumId(albumId)
{
    ui->setupUi(this);
    this->resize(CardWidget::width, CardWidget::width + 36);
    ui->icoLabel->resize(CardWidget::width, CardWidget::width);
    ui->icoLabel->setAlignment(Qt::AlignCenter);
    QPixmap img = icoPixmap.scaled(CardWidget::width, CardWidget::width, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui->icoLabel->setPixmap(img);
    ui->icoLabel->installEventFilter(this);

    ui->titleLabel->setText(QString("<a style='color: #DCDCDC; text-decoration:none;' href='search://album=%1'>%2</a>").arg(m_albumId, title) );

    if (artistName.isEmpty())
        ui->artistLabel->setVisible(false);
    else {
        ui->artistLabel->setVisible(true);
        ui->artistLabel->setText(QString("<a style='color: #DCDCDC; text-decoration:none;' href='search://artist=%1'>%1</a>").arg(artistName));
    }

    m_playFrame = new FloatFrame(this);
    m_playFrame->setFillBackground(false);
    m_playFrame->setHideInterval(10);

    connect(ui->titleLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkClicked(const QString &)));
    connect(ui->artistLabel, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkClicked(const QString &)));
    connect(m_playFrame, SIGNAL(mouseClicked()), this, SLOT(iconClicked()));

    QPixmap imgPlay(":/images/playMask.png");

    m_label = new QLabel(m_playFrame);

    m_label->resize(imgPlay.width(), imgPlay.height());
    m_label->setPixmap(imgPlay);
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
    emit playIconClicked(m_albumId);
}



