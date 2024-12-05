#include "cardlistwidget.h"
#include "ui_cardlistwidget.h"

#include "cardwidget.h"
#include "iconhelper.h"

#include <QScrollBar>
#include <QPropertyAnimation>


const int CardListWidget::s_cardSpace = 12;
const int CardListWidget::s_cardWidth = CardWidget::width + CardListWidget::s_cardSpace;

CardListWidget::CardListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CardListWidget)
{
    ui->setupUi(this);
    ui->hLayout->setSpacing(CardListWidget::s_cardSpace);

    IconHelper::Instance()->setIcon(ui->leftPushButton, QChar(0xF0d9), 16);
    IconHelper::Instance()->setIcon(ui->rightPushButton, QChar(0xF0da), 16);

    m_scrollBar = ui->scrollArea->horizontalScrollBar();
    m_animation = new QPropertyAnimation(m_scrollBar, "value", this);

    QObject::connect(ui->rightPushButton, &QPushButton::clicked, this, &CardListWidget::onRightShift);
    QObject::connect(ui->leftPushButton, &QPushButton::clicked, this, &CardListWidget::onLeftShift);

    QObject::connect(m_animation, &QPropertyAnimation::finished, this, &CardListWidget::adjustButtonState);

    QObject::connect(m_scrollBar, &QScrollBar::rangeChanged, this, &CardListWidget::adjustButtonState);
}

CardListWidget::~CardListWidget()
{
    delete ui;
}

void CardListWidget::setTitle(const QString &title)
{
    ui->titleLabel->setText(title);
}

void CardListWidget::addCard(CardWidget *card)
{
    ui->hLayout->insertWidget(ui->hLayout->count()-1, card);
}

void CardListWidget::onRightShift()
{
    if(m_animation->state() != QAbstractAnimation::Stopped)
        m_animation->stop();

    int _startValue = m_scrollBar->value();
    int _pageStep = m_scrollBar->pageStep() - m_scrollBar->pageStep() % ( CardListWidget::s_cardWidth );
    int _pos = _startValue + _pageStep;

    m_animation->setDuration(500);
    m_animation->setStartValue(_startValue);
    m_animation->setEndValue(_pos);

    m_animation->start();

}


void CardListWidget::onLeftShift()
{

    if(m_animation->state() != QAbstractAnimation::Stopped)
        m_animation->stop();

    int _startValue = m_scrollBar->value();
    int _pageStep = m_scrollBar->pageStep() - m_scrollBar->pageStep() % ( CardListWidget::s_cardWidth );
    int _pos = _startValue - _pageStep;

    m_animation->setDuration(500);
    m_animation->setStartValue(_startValue);
    m_animation->setEndValue(_pos);

    m_animation->start();

}

void CardListWidget::adjustButtonState()
{

    if (m_scrollBar->value() == m_scrollBar->minimum())
        ui->leftPushButton->setEnabled(false);
    else
        ui->leftPushButton->setEnabled(true);

    if (m_scrollBar->value() == m_scrollBar->maximum())
        ui->rightPushButton->setEnabled(false);
    else
        ui->rightPushButton->setEnabled(true);

}

