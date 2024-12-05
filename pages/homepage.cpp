#include "homepage.h"
#include "ui_homepage.h"
#include "CardWidget/cardwidget.h"
#include "mediahelper.h"
#include "config.h"

HomePage::HomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);

    ui->recentWidget->setTitle(QCoreApplication::translate("HomePage", "Recently Played", nullptr));
    ui->newestWidget->setTitle(QCoreApplication::translate("HomePage", "Recently Added", nullptr));
    ui->frequentWidget->setTitle(QCoreApplication::translate("HomePage", "Most Played", nullptr));
    ui->randomWidget->setTitle(QCoreApplication::translate("HomePage", "Random", nullptr));

    initData();
}

HomePage::~HomePage()
{
    delete ui;
}

void HomePage::initData()
{

    AlbumList recentAL = MediaHelper::Instance()->getAlbumList(MediaHelper::Recent);

    Config::G_Debug( "AlbumList size:", recentAL.size());

    for (int i = 0; i < recentAL.count(); i++) {
        CardWidget *cw = new CardWidget(recentAL.at(i).id(), QPixmap(recentAL.at(i).posterPath()), recentAL.at(i).name(), recentAL.at(i).artist() , this);
        connect(cw, SIGNAL(linkClicked(const QString &)), this, SIGNAL(linkClicked(const QString &)));
        connect(cw, SIGNAL(playIconClicked(const QString &)), this, SIGNAL(playAlbum(const QString &)));
        ui->recentWidget->addCard( cw );
    }

    AlbumList newestAL = MediaHelper::Instance()->getAlbumList(MediaHelper::Newest);

    Config::G_Debug( "AlbumList size:", newestAL.size());

    for (int i = 0; i < newestAL.count(); i++) {
        CardWidget *cw = new CardWidget(newestAL.at(i).id(), QPixmap(newestAL.at(i).posterPath()),  newestAL.at(i).name(), newestAL.at(i).artist() , this);
        connect(cw, SIGNAL(linkClicked(const QString &)), this, SIGNAL(linkClicked(const QString &)));
        connect(cw, SIGNAL(playIconClicked(const QString &)), this, SIGNAL(playAlbum(const QString &)));
        ui->newestWidget->addCard(cw );
    }

    AlbumList frequentAL = MediaHelper::Instance()->getAlbumList(MediaHelper::Frequent);

    Config::G_Debug( "AlbumList size:", frequentAL.size());

    for (int i = 0; i < frequentAL.count(); i++) {
        CardWidget *cw = new CardWidget(frequentAL.at(i).id(), QPixmap(frequentAL.at(i).posterPath()),  frequentAL.at(i).name(), frequentAL.at(i).artist() , this);
        connect(cw, SIGNAL(linkClicked(const QString &)), this, SIGNAL(linkClicked(const QString &)));
        connect(cw, SIGNAL(playIconClicked(const QString &)), this, SIGNAL(playAlbum(const QString &)));
        ui->frequentWidget->addCard(cw );
    }

    AlbumList randomAL = MediaHelper::Instance()->getAlbumList(MediaHelper::Random);

    Config::G_Debug( "AlbumList size:", randomAL.size());

    for (int i = 0; i < randomAL.count(); i++) {
        CardWidget *cw = new CardWidget(randomAL.at(i).id(), QPixmap(randomAL.at(i).posterPath()),  randomAL.at(i).name(), randomAL.at(i).artist() , this);
        connect(cw, SIGNAL(linkClicked(const QString &)), this, SIGNAL(linkClicked(const QString &)));
        connect(cw, SIGNAL(playIconClicked(const QString &)), this, SIGNAL(playAlbum(const QString &)));
        ui->randomWidget->addCard(cw );
    }

}
