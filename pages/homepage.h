#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

namespace Ui {
class HomePage;
}

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage();

    void initData();

signals:
    void linkClicked(const QString &link);
    void playAlbum(const QString &albumId);
    void artistClicked(const QString &artist);
    void albumClicked(const QString &album);
private:
    Ui::HomePage *ui;
};

#endif // HOMEPAGE_H
