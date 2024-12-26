#ifndef ARTISTINFOPAGE_H
#define ARTISTINFOPAGE_H

#include "artistobject.h"

#include <QWidget>

namespace Ui {
class ArtistInfoPage;
}

class ArtistInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit ArtistInfoPage(QWidget *parent = nullptr);
    ~ArtistInfoPage();

    void loadArtist(const QString &id);

signals:
    void linkActivated(const QString &link);
    void playAlbum(const QString &albumId);

private:
    Ui::ArtistInfoPage *ui;

    ArtistObject m_artistObject;
};

#endif // ARTISTINFOPAGE_H
