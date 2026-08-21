#ifndef ALBUMWIDGET_H
#define ALBUMWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QVector>
#include <QMouseEvent>
#include <QHash>
#include <QSet>
#include <QResizeEvent>
#include "../models/album.h"
#include "../models/song.h"

// Forward declarations
class ClickableWidget;
class SubsonicApi;
class CacheManager;

class AlbumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumWidget(QWidget *parent = nullptr);
    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setAlbums(const QVector<Album> &albums);
    void clear();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void albumSelected(const Album &album);
    void playAlbumRequested(const Album &album);
    void songSelected(const Song &song);

public slots:
    void handleAlbumClick(int index);
    void onCoverArtReceived(const QString &coverArtId, const QByteArray &data);

private:
    void setupUI();
    void updateUI();
    int computeColumns() const;
    void loadAlbumCover(QLabel *coverLabel, const QString &coverArtId);
    void requestVisibleCovers();
    bool isVisibleInViewport(QWidget *widget) const;

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QGridLayout *m_gridLayout;
    QVector<Album> m_albums;
    QHash<QString, QLabel *> m_coverLabels;
    QSet<QString> m_pendingCoverArtIds;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
};

// Clickable widget for album items
class ClickableWidget : public QWidget
{
    Q_OBJECT

public:
    ClickableWidget(int index, QWidget *parent = nullptr) : QWidget(parent), m_index(index) {}

signals:
    void clicked(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        Q_UNUSED(event);
        emit clicked(m_index);
    }

private:
    int m_index;
};

#endif // ALBUMWIDGET_H
