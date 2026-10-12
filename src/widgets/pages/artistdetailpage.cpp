#include "artistdetailpage.h"

#include "widgets/components/coverlabel.h"
#include "widgets/pages/albumsview.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr int kAvatarSize = 160;
}

ArtistDetailPage::ArtistDetailPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    auto *header = new QHBoxLayout();
    header->setSpacing(16);

    m_avatar = new CoverLabel(this);
    m_avatar->setFixedSize(kAvatarSize, kAvatarSize);
    m_avatar->setRadius(8);              // 方形倒角（与专辑详情封面一致）
    m_avatar->setHoverPlayEnabled(false);

    auto *infoLayout = new QVBoxLayout();
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName(QStringLiteral("artistDetailName"));
    QFont nameFont = m_nameLabel->font();
    nameFont.setPointSize(nameFont.pointSize() + 8);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);

    m_descLabel = new QLabel(this);
    m_descLabel->setObjectName(QStringLiteral("artistDetailDesc"));

    m_favoriteButton = new QPushButton(this);
    m_favoriteButton->setCursor(Qt::PointingHandCursor);
    m_favoriteButton->setFixedWidth(100);

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_descLabel);
    infoLayout->addWidget(m_favoriteButton);
    infoLayout->addStretch();

    header->addWidget(m_avatar);
    header->addLayout(infoLayout, 1);

    m_albumsView = new AlbumsView(this);
    m_albumsView->setSource(AlbumsView::Source::Artist);

    layout->addLayout(header);
    layout->addWidget(m_albumsView, 1);

    connect(m_favoriteButton, &QPushButton::clicked, this, [this]() {
        const bool favorite = !m_artist.isFavorite();
        m_artist.setFavorite(favorite);
        m_favoriteButton->setText(favorite ? QStringLiteral("已收藏") : QStringLiteral("收藏"));
        Q_EMIT favoriteToggled(m_artist, favorite);
    });
    connect(m_albumsView, &AlbumsView::albumClicked, this, &ArtistDetailPage::albumClicked);
    connect(m_albumsView, &AlbumsView::playAlbumRequested, this,
            &ArtistDetailPage::playAlbumRequested);

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api()) {
        connect(ctx->api(), &SubsonicApi::artistReady, this,
                [this](const Artist &artist, const QVector<Album> &albums) {
                    if (artist.id() != m_artist.id())
                        return;
                    m_artist = artist;
                    m_nameLabel->setText(artist.name());
                    m_descLabel->setText(QStringLiteral("%1 张专辑").arg(qMax(artist.albumCount(),
                                                                              albums.size())));
                    updateAvatar();
                    m_favoriteButton->setText(artist.isFavorite() ? QStringLiteral("已收藏")
                                                                  : QStringLiteral("收藏"));
                });
    }
}

void ArtistDetailPage::updateAvatar()
{
    // kAvatarSize 为逻辑显示尺寸，CoverLabel 会按设备像素比取样（Retina 下取 320）
    m_avatar->setCoverArt(m_artist.coverArtId(), kAvatarSize);
}

void ArtistDetailPage::setArtist(const Artist &artist)
{
    m_artist = artist;
    m_nameLabel->setText(artist.name());
    m_descLabel->setText(QStringLiteral("%1 张专辑").arg(artist.albumCount()));
    m_avatar->setPlaceholderText(artist.name());
    updateAvatar();
    m_favoriteButton->setText(artist.isFavorite() ? QStringLiteral("已收藏") : QStringLiteral("收藏"));

    m_albumsView->setArtist(artist);
    m_albumsView->refresh();

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api() && ctx->api()->isConfigured())
        ctx->api()->getArtist(artist.id());
}

void ArtistDetailPage::refresh()
{
    m_albumsView->refresh();
}

void ArtistDetailPage::retranslate()
{
    m_favoriteButton->setText(m_artist.isFavorite() ? QStringLiteral("已收藏")
                                                    : QStringLiteral("收藏"));
}
