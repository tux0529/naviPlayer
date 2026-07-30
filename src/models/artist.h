#ifndef ARTIST_H
#define ARTIST_H

#include <QString>

class Artist
{
public:
    Artist();

    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    QString coverArtId() const { return m_coverArtId; }
    void setCoverArtId(const QString &coverArtId) { m_coverArtId = coverArtId; }

    int albumCount() const { return m_albumCount; }
    void setAlbumCount(int albumCount) { m_albumCount = albumCount; }

    bool isFavorite() const { return m_isFavorite; }
    void setFavorite(bool favorite) { m_isFavorite = favorite; }

private:
    QString m_id;
    QString m_name;
    QString m_coverArtId;
    int m_albumCount;
    bool m_isFavorite;
};

#endif // ARTIST_H
