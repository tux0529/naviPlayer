#include "homepage.h"

#include "widgets/components/albumrowscroller.h"
#include "core/appcontext.h"
#include "core/subsonicapi.h"
#include "core/databasemanager.h"
#include "core/appsettings.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
constexpr int kSectionSize = 20;
}

HomePage::HomePage(QWidget *parent)
    : PageBase(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_content = new QWidget(scroll);
    m_layout = new QVBoxLayout(m_content);
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->setSpacing(16);

    // 先放 stretch，各分组插入到它之前
    m_layout->addStretch(1);
    scroll->setWidget(m_content);
    outer->addWidget(scroll);

    buildSection(QStringLiteral("newest"), QStringLiteral("最近添加的专辑"));
    buildSection(QStringLiteral("frequent"), QStringLiteral("播放最多的专辑"));
    buildSection(QStringLiteral("recent"), QStringLiteral("最近播放的专辑"));
    buildSection(QStringLiteral("random"), QStringLiteral("随机专辑"));

    AppContext *ctx = AppContext::instance();
    if (ctx && ctx->api())
        connect(ctx->api(), &SubsonicApi::albumListReady, this, &HomePage::onAlbumListReady);
}

void HomePage::buildSection(const QString &key, const QString &title)
{
    HomeSection section;
    section.key = key;
    section.title = title;

    section.row = new AlbumRowScroller(m_content);
    section.row->setTitle(title);
    connect(section.row, &AlbumRowScroller::albumClicked, this, &HomePage::albumClicked);
    connect(section.row, &AlbumRowScroller::artistClicked, this, &HomePage::artistClicked);
    connect(section.row, &AlbumRowScroller::playClicked, this, &HomePage::playAlbumRequested);

    section.loadingLabel = new QLabel(QStringLiteral("加载中…"), m_content);
    section.loadingLabel->setObjectName(QStringLiteral("sectionLoading"));

    auto *wrapper = new QWidget(m_content);
    auto *wl = new QVBoxLayout(wrapper);
    wl->setContentsMargins(0, 0, 0, 0);
    wl->setSpacing(4);
    wl->addWidget(section.row);
    wl->addWidget(section.loadingLabel);

    // 插入到 stretch 之前
    m_layout->insertWidget(m_layout->count() - 1, wrapper);

    m_sections.append(section);
}

void HomePage::retranslate()
{
    const QStringList titles = {QStringLiteral("最近添加的专辑"), QStringLiteral("播放最多的专辑"),
                                QStringLiteral("最近播放的专辑"), QStringLiteral("随机专辑")};
    for (int i = 0; i < m_sections.size() && i < titles.size(); ++i) {
        m_sections[i].title = titles.at(i);
        m_sections[i].row->setTitle(titles.at(i));
    }
}

void HomePage::applyTtlCache()
{
    DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
    AppSettings *settings = AppContext::instance() ? AppContext::instance()->settings() : nullptr;
    if (!db)
        return;

    const int ttlMinutes = settings ? settings->pageCacheTtlMinutes() : 30;

    for (const HomeSection &s : m_sections) {
        const QString payload = db->loadPageCache(QStringLiteral("home_%1").arg(s.key), ttlMinutes);
        if (payload.isEmpty())
            continue;
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isArray())
            continue;
        QVector<Album> albums;
        for (const QJsonValue &v : doc.array()) {
            const Album a = Album::fromJson(v.toObject());
            if (a.isValid())
                albums.append(a);
        }
        if (albums.isEmpty())
            continue;
        s.row->setAlbums(albums);
        s.loadingLabel->hide();
    }
}

void HomePage::showSectionFromLocal(const HomeSection &section)
{
    // recent / frequent 优先使用本地播放历史（服务端无播放数据时也能显示）
    DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
    if (!db)
        return;

    QVector<Album> albums;
    if (section.key == QStringLiteral("recent"))
        albums = db->recentPlayedAlbums(kSectionSize);
    else if (section.key == QStringLiteral("frequent"))
        albums = db->mostPlayedAlbums(kSectionSize);
    else
        return;

    if (!albums.isEmpty()) {
        section.row->setAlbums(albums);
        section.loadingLabel->hide();
    }
}

void HomePage::fetchSection(const HomeSection &section)
{
    // 先显示本地兜底数据（播放历史）
    showSectionFromLocal(section);

    AppContext *ctx = AppContext::instance();
    if (!ctx || !ctx->api() || !ctx->api()->isConfigured())
        return;

    if (section.key == QStringLiteral("newest"))
        ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::Newest, kSectionSize, 0);
    else if (section.key == QStringLiteral("frequent"))
        ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::Frequent, kSectionSize, 0);
    else if (section.key == QStringLiteral("recent"))
        ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::Recent, kSectionSize, 0);
    else if (section.key == QStringLiteral("random"))
        ctx->api()->getAlbumList2(SubsonicApi::AlbumListType::Random, kSectionSize, 0);
}

void HomePage::refresh()
{
    ++m_generation;
    m_requestGeneration = m_generation;

    applyTtlCache();
    for (const HomeSection &s : m_sections)
        fetchSection(s);
}

void HomePage::onAlbumListReady(const QString &type, int offset, const QVector<Album> &albums)
{
    if (offset != 0)
        return;
    if (m_generation != m_requestGeneration)
        return;     // 页面已刷新，忽略过期响应

    for (HomeSection &s : m_sections) {
        if (s.key != type)
            continue;

        if (albums.isEmpty()) {
            // 保留本地播放历史兜底；没有兜底时给出提示
            if (s.row && s.row->isVisible() && s.loadingLabel
                && s.loadingLabel->text() != QStringLiteral("暂无数据")) {
                s.loadingLabel->setText(QStringLiteral("暂无数据"));
            }
            continue;
        }

        s.row->setAlbums(albums);
        s.loadingLabel->hide();

        DatabaseManager *db = AppContext::instance() ? AppContext::instance()->database() : nullptr;
        if (db) {
            QJsonArray arr;
            for (const Album &a : albums)
                arr.append(a.toJson());
            db->savePageCache(QStringLiteral("home_%1").arg(type),
                              QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact)));
        }
    }
}
