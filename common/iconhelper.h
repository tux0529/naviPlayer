#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QLabel>
#include <QMutex>
#include <QAbstractButton>

class IconHelper : public QObject
{
public:
    explicit IconHelper(QObject *parent = 0);
    static IconHelper *Instance()
    {
        static QMutex mutex;

        if (!self) {
            QMutexLocker locker(&mutex);

            if (!self) {
                self = new IconHelper;
            }
        }

        return self;
    }

    void setIcon(QLabel *lab, QChar c, quint32 size = 9);
    void setIcon(QAbstractButton *btn, QChar c, quint32 size = 9);

private:
    static IconHelper *self;    //对象自身
    QFont iconFont;             //图形字体
};


#endif // ICONHELPER_H
