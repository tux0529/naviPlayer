#include "searchwidget.h"
#include "songlistwidget.h"
#include "albumwidget.h"
#include "artistwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

SearchWidget::SearchWidget(QWidget *parent)
    : QWidget(parent)
    , m_subsonicApi(nullptr)
    , m_cacheManager(nullptr)
{
    setupUI();
}

void SearchWidget::setSubsonicApi(SubsonicApi *api)
{
    m_subsonicApi = api;
    if (m_albumResults) {
        m_albumResults->setSubsonicApi(api);
    }
}

void SearchWidget::setCacheManager(CacheManager *cache)
{
    m_cacheManager = cache;
    if (m_albumResults) {
        m_albumResults->setCacheManager(cache);
    }
}

void SearchWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);

    // Search input area
    QWidget *searchArea = new QWidget(this);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchArea);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("搜索歌曲、专辑、艺术家...");
    m_searchInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #2d2d30;
            color: #ffffff;
            border: 1px solid #3d3d3f;
            border-radius: 8px;
            padding: 10px 16px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1px solid #3a80b3;
        }
    )");
    searchLayout->addWidget(m_searchInput);

    QPushButton *searchBtn = new QPushButton("搜索", this);
    searchBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3a80b3;
            color: #ffffff;
            border: none;
            border-radius: 8px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #4a90c3;
        }
    )");
    searchLayout->addWidget(searchBtn);

    mainLayout->addWidget(searchArea);

    // Results tab widget
    m_resultsTab = new QTabWidget(this);
    m_resultsTab->setStyleSheet(R"(
        QTabWidget::pane {
            background-color: #2d2d30;
            border: none;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: #3d3d3f;
            color: #aaaaaa;
            padding: 10px 20px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            margin-right: 4px;
        }
        QTabBar::tab:selected {
            background-color: #2d2d30;
            color: #ffffff;
        }
        QTabBar::tab:hover:!selected {
            background-color: #4d4d4f;
            color: #ffffff;
        }
    )");

    // Songs tab
    m_songResults = new SongListWidget(this);
    m_resultsTab->addTab(m_songResults, "歌曲");

    // Albums tab
    m_albumResults = new AlbumWidget(this);
    m_resultsTab->addTab(m_albumResults, "专辑");

    // Artists tab
    m_artistResults = new ArtistWidget(this);
    m_resultsTab->addTab(m_artistResults, "艺术家");

    mainLayout->addWidget(m_resultsTab);

    // No results label
    m_noResultsLabel = new QLabel("请输入关键词搜索音乐", this);
    m_noResultsLabel->setAlignment(Qt::AlignCenter);
    m_noResultsLabel->setStyleSheet(R"(
        color: #888888;
        font-size: 16px;
        padding: 40px;
    )");

    // Connections
    connect(searchBtn, &QPushButton::clicked, this, [this]() {
        onSearchTextChanged(m_searchInput->text());
    });
    connect(m_searchInput, &QLineEdit::returnPressed, this, [this]() {
        onSearchTextChanged(m_searchInput->text());
    });

    connect(m_songResults, &SongListWidget::songSelected, this, &SearchWidget::songSelected);
    connect(m_songResults, &SongListWidget::songFavoriteClicked, this, &SearchWidget::songFavoriteClicked);
    connect(m_albumResults, &AlbumWidget::albumSelected, this, &SearchWidget::albumSelected);
    connect(m_artistResults, &ArtistWidget::artistSelected, this, &SearchWidget::artistSelected);
}

void SearchWidget::onSearchTextChanged(const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }

    if (m_subsonicApi) {
        m_subsonicApi->search3(text);
    }
}

void SearchWidget::onSearchResultsReceived(const QVector<Artist> &artists,
                                            const QVector<Album> &albums,
                                            const QVector<Song> &songs)
{
    m_songResults->setSongs(songs);
    m_albumResults->setAlbums(albums);
    m_artistResults->setArtists(artists);

    if (songs.isEmpty() && albums.isEmpty() && artists.isEmpty()) {
        m_noResultsLabel->setText("没有找到匹配的结果");
    }
}
