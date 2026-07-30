#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QMap>
#include <QMetaObject>
#include <QVector>
#include <QWidget>
#include "../models/album.h"

class CacheManager;
class SubsonicApi;

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    enum SectionType {
        RecentAlbums,
        NewestAlbums,
        FrequentAlbums,
        RandomAlbums
    };

    explicit HomeWidget(QWidget *parent = nullptr);

    void setSubsonicApi(SubsonicApi *api);
    void setCacheManager(CacheManager *cache);
    void setSectionLoading(SectionType type, bool loading);
    void setSectionAlbums(SectionType type, const QVector<Album> &albums);
    void setSectionMessage(SectionType type, const QString &message, bool isError = false);
    void resetSections();

signals:
    void albumSelected(const Album &album);
    void playAlbumRequested(const Album &album);

private:
    void setupUI();
    QWidget *section(SectionType type) const;

    QMap<int, QWidget *> m_sections;
    SubsonicApi *m_subsonicApi;
    CacheManager *m_cacheManager;
    QMetaObject::Connection m_coverArtConnection;
};

#endif // HOMEWIDGET_H
