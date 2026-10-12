#include "playlistspage.h"

#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

PlaylistsPage::PlaylistsPage(QWidget *parent)
    : PageBase(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    m_list = new QListWidget(this);
    m_list->setObjectName(QStringLiteral("playlistList"));

    auto *buttonLayout = new QHBoxLayout();
    m_newButton = new QPushButton(QStringLiteral("新建"), this);
    m_renameButton = new QPushButton(QStringLiteral("重命名"), this);
    m_deleteButton = new QPushButton(QStringLiteral("删除"), this);
    buttonLayout->addWidget(m_newButton);
    buttonLayout->addWidget(m_renameButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addStretch();

    layout->addWidget(m_list, 1);
    layout->addLayout(buttonLayout);

    connect(m_newButton, &QPushButton::clicked, this, &PlaylistsPage::onNewPlaylist);
    connect(m_renameButton, &QPushButton::clicked, this, &PlaylistsPage::onRenamePlaylist);
    connect(m_deleteButton, &QPushButton::clicked, this, &PlaylistsPage::onDeletePlaylist);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &PlaylistsPage::onItemDoubleClicked);

    AppContext *ctx = AppContext::instance();
    if (ctx) {
        connect(ctx, &AppContext::playlistsChanged, this, [this]() {
            updateList(AppContext::instance()->playlists());
        });
        connect(ctx->api(), &SubsonicApi::playlistChanged, this, [this](const QString &, bool ok) {
            if (ok && AppContext::instance()->api())
                AppContext::instance()->api()->getPlaylists();
        });
    }
}

void PlaylistsPage::updateList(const QVector<Playlist> &playlists)
{
    m_playlists = playlists;
    const QString selectedId = currentPlaylistId();

    m_list->clear();
    for (const Playlist &p : playlists) {
        auto *item = new QListWidgetItem(QStringLiteral("%1（%2 首）").arg(p.name()).arg(p.songCount()));
        item->setData(Qt::UserRole, p.id());
        m_list->addItem(item);
        if (p.id() == selectedId)
            m_list->setCurrentItem(item);
    }
}

QString PlaylistsPage::currentPlaylistId() const
{
    QListWidgetItem *item = m_list->currentItem();
    return item ? item->data(Qt::UserRole).toString() : QString();
}

void PlaylistsPage::onNewPlaylist()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("新建播放列表"),
                                               QStringLiteral("名称："), QLineEdit::Normal,
                                               QString(), &ok);
    if (!ok || name.trimmed().isEmpty())
        return;

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api() && ctx->api()->isConfigured())
        ctx->api()->createPlaylist(name.trimmed());
}

void PlaylistsPage::onRenamePlaylist()
{
    const QString id = currentPlaylistId();
    if (id.isEmpty())
        return;

    QString currentName;
    for (const Playlist &p : m_playlists) {
        if (p.id() == id) {
            currentName = p.name();
            break;
        }
    }

    bool ok = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("重命名播放列表"),
                                               QStringLiteral("名称："), QLineEdit::Normal,
                                               currentName, &ok);
    if (!ok || name.trimmed().isEmpty() || name == currentName)
        return;

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api())
        ctx->api()->updatePlaylist(id, name.trimmed());
}

void PlaylistsPage::onDeletePlaylist()
{
    const QString id = currentPlaylistId();
    if (id.isEmpty())
        return;

    if (QMessageBox::question(this, QStringLiteral("删除播放列表"),
                              QStringLiteral("确定要删除该播放列表吗？"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api())
        ctx->api()->deletePlaylist(id);
}

void PlaylistsPage::onItemDoubleClicked()
{
    const QString id = currentPlaylistId();
    if (id.isEmpty())
        return;
    for (const Playlist &p : m_playlists) {
        if (p.id() == id) {
            Q_EMIT playlistOpened(p);
            return;
        }
    }
}

void PlaylistsPage::refresh()
{
    AppContext *ctx = AppContext::instance();
    if (!ctx)
        return;

    if (ctx->api() && ctx->api()->isConfigured()) {
        ctx->refreshPlaylists();
    } else if (ctx->database()) {
        updateList(ctx->database()->playlists());
    }
}

void PlaylistsPage::retranslate()
{
    m_newButton->setText(QStringLiteral("新建"));
    m_renameButton->setText(QStringLiteral("重命名"));
    m_deleteButton->setText(QStringLiteral("删除"));
}
