#include "widget.h"
#include "ui_widget.h"
#include "cardwidget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    QPixmap img(":/images/getCoverArt.jpeg");

    ui->widget->setTitle("Recently Played");
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo1", "Project1", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo2", "Project2", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo3", "Project3", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo4", "Project4", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo5", "Project3", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo6", "Project2", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo7", "Project3", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo8", "Project2", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo9", "Project3", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo1", "Project1", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo2", "Project2", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo3", "Project3", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo4", "Project4", this) );
    ui->widget->addCard(new CardWidget(img, "Nextcloud CardDemo5", "Project3", this) );

    ui->widget2->setTitle("Random");
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo6", "Project2", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo7", "Project3", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo8", "Project2", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo9", "Project3", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo1", "Project1", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo2", "Project2", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo3", "Project3", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo4", "Project4", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo5", "Project3", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo6", "Project2", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo7", "Project3", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo8", "Project2", this) );
    ui->widget2->addCard(new CardWidget(img, "Nextcloud CardDemo9", "Project3", this) );
}

Widget::~Widget()
{
    delete ui;
}

