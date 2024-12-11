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

private:

    QString m_id;
    QString m_name;

};

#endif // MEDIABASE_H
