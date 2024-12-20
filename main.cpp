#include "mainwidget.h"
#include "servermanagewidget.h"
#include "application.h"
#include "config.h"

#include <QApplication>

/******************
 *  homepage
 *  favorpage
 *  albumpage
 *  artistpage
 *  genrespage
 *  albumInfoPage
 ******************/


int main(int argc, char *argv[])
{
    Application a(argc, argv);

    if (!Config::Instance()->serverSeted()){

        Application::instance()->showSeverManager();
        /*
        ServerManageWidget smw;

        if (smw.exec() == QDialog::Accepted ){
            smw.close();
            MainWidget w;
            w.show();
            return a.exec();
        }
*/
    }
    else{

        Application::instance()->showMainWindow();
    }

    return a.exec();

}
