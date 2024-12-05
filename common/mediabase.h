#ifndef MEDIABASE_H
#define MEDIABASE_H


#include <QString>
#include <QIcon>

class MediaBase
{
public:
    MediaBase();

    const QString &id() const{ return this->m_id; }
    void setId(const QString &id);

    const QString &name() const{ return this->m_name; }
    void setName(const QString &name);

    const QString &coverId() const{ return this->m_coverId; }
    void setCoverId(const QString &coverId);

    void setArtDirpath(const QString &dirpath);

    QString coverPath() const;
    QString posterPath() const;
    QString iconPath() const;

    void setIcon(const QString &coverPath);
    const QIcon &icon() const { return this->m_icon; };

private:

    QString m_id;
    QString m_name;
    QString m_coverId;

    QString m_artDirpath;

    QIcon m_icon;
};

#endif // MEDIABASE_H
