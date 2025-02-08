#include "artistinfopage.h"
#include "ui_artistinfopage.h"

#include "CardWidget/cardwidget.h"

#include "mediahelper.h"
#include "iconhelper.h"





ArtistInfoPage::ArtistInfoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ArtistInfoPage)
{
    ui->setupUi(this);

    ui->albumCardList->setTitle(QCoreApplication::translate("ArtistInfoPage", "Albums", nullptr));

    IconHelper::Instance()->setIcon(ui->playButton, QChar(0xF144), 36);
    IconHelper::Instance()->setIcon(ui->appendToQueueEndButton, QChar(0xF067), 24);
    IconHelper::Instance()->setIcon(ui->insertToQueueButton, QChar(0xF055), 24);
    IconHelper::Instance()->setIcon(ui->favorButton, QChar(0xF08a), 24);
}

ArtistInfoPage::~ArtistInfoPage()
{
    delete ui;
}

void ArtistInfoPage::loadArtist(const QString &id)
{
    m_artistObject = MediaHelper::Instance()->getArtist(id);
    ui->posterLabel->setPixmap(QPixmap(MediaHelper::Instance()->getCoverArt(m_artistObject.id(), MediaHelper::ArtistCover, MediaHelper::Poster)));
    ui->pageTitleLabel->setText(m_artistObject.name());
    ui->pageInfoLabel->setText(tr("Artist · %1 Albums ")
                                   .arg(m_artistObject.albumCount() ));

    ui->albumCardList->clear();
    for(int i = m_artistObject.albumCount(); i > 0; --i){
        const Album &a = m_artistObject.at(i-1);
        CardWidget *cw = new CardWidget(a, this);
        connect(cw, SIGNAL(linkActivated(const QString &)), this, SIGNAL(linkActivated(const QString &)));
        connect(cw, SIGNAL(playIconClicked(const QString &)), this, SIGNAL(playAlbum(const QString &)));
        ui->albumCardList->addCard( cw );
    }

}
