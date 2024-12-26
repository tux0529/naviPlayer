#include "application.h"
#include "mainwidget.h"
#include "config.h"

#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QTranslator>
#include <QFont>
#include <QFile>
#include <QColor>
#include <QPalette>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QDesktopWidget>
#else
#include <QScreen>
#endif
#include <QDebug>


MainWidget *Application::s_mainWindow = nullptr;
ServerManageWidget *Application::s_serverManager = nullptr;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv),
      m_systemMenu(nullptr)
{

    QCoreApplication::setOrganizationName("cn.redtux");
    QCoreApplication::setOrganizationDomain("redtux.cn");
    QCoreApplication::setApplicationName("naviPlayer");
    QCoreApplication::setApplicationVersion(QLatin1String("0.0.1"));

    this->setLanguage();

    this->setTheme("black");

    this->initTrayMenu();

}

Application::~Application()
{
    delete s_mainWindow;
    delete s_serverManager;
    delete m_systemMenu;
}

Application *Application::instance()
{
    return (qobject_cast<Application *>(QCoreApplication::instance()));
}

void Application::setLanguage(const QString &language)
{

    QTranslator translator;

    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "mediaConverter_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            this->installTranslator(&translator);
            break;
        }
    }
}

void Application::setTheme(const QString &theme)
{
    QFont font = this->font();

    //
#ifdef Q_OS_MACOS
    font.setPointSize(14);
#else
    font.setPointSize(9);
#endif

    this->setFont(font);

    QString qssFile = QString(":/theme/%1.css").arg(theme.toLower());

    QFile file(qssFile);

    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }

    if(s_mainWindow){
        s_mainWindow->repaint();
    }

}

void Application::setFormInCenter(QWidget *frm)
{
    int frmX = frm->width();
    int frmY = frm->height();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QRect rect = Application::primaryScreen()->geometry();
#else
    QRect rect = QApplication::desktop()->availableGeometry();
#endif

    QPoint movePoint(rect.width() / 2 - frmX / 2, rect.height() / 2 - frmY / 2);
    frm->move(movePoint);
}

int Application::screenWidth()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QRect rect = Application::primaryScreen()->geometry();
#else
    QRect rect = QApplication::desktop()->availableGeometry();
#endif

    return rect.width();
}

void Application::initTrayMenu(){

    QSystemTrayIcon *m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(Application::instance()->systemMenu());
    m_trayIcon->setIcon(QIcon(":/images/Musicpl.png"));
    m_trayIcon->setVisible(true);
}

QMenu *Application::systemMenu()
{
    if( !m_systemMenu){
        m_systemMenu = new QMenu();

        QAction *showAct = new QAction( tr("&Show Main Widow"), this);
        connect(showAct, SIGNAL(triggered()), Application::instance(), SLOT(showMainWindow()));
        m_systemMenu->addAction( showAct);

        QAction *systemPreferencesAct = new QAction( tr("&Preferences"), this);
        connect(systemPreferencesAct, SIGNAL(triggered()), Application::instance(), SLOT(showSystemPreferences()));
        m_systemMenu->addAction( systemPreferencesAct);

        QAction *jobManagerAct = new QAction( tr("&Manage Servers"), this);
        connect(jobManagerAct, SIGNAL(triggered()), Application::instance(), SLOT(showSeverManager()));
        m_systemMenu->addAction( jobManagerAct);

        QAction *aboutAct = new QAction( tr("&About"), this);
        connect(aboutAct, SIGNAL(triggered()), Application::instance(), SLOT(showSystemPreferences()));
        m_systemMenu->addAction( aboutAct);

        m_systemMenu->addSeparator();

        QAction *exitAct = new QAction( tr("&Exit"), this);
        connect(exitAct, SIGNAL(triggered()), Application::instance(), SLOT(quit()));
        m_systemMenu->addAction( exitAct);
    }
    return m_systemMenu;
}

void Application::showMainWindow(){
    Application::mianWindow()->show();
    Application::mianWindow()->raise();
}

void Application::showSystemPreferences()
{

}

void Application::showSeverManager()
{
    static QMutex mutex2;
    if (!s_serverManager) {
        QMutexLocker locker(&mutex2);
        if (!s_serverManager) {
            s_serverManager = new ServerManageWidget();

            connect(s_serverManager, SIGNAL(accept()), Application::instance(), SLOT(showMainWindow()));
        }
    }
    s_serverManager->show();
    s_serverManager->raise();
}

MainWidget *Application::mianWindow()
{
    static QMutex mutex;
    if (!s_mainWindow) {
        QMutexLocker locker(&mutex);
        if (!s_mainWindow) {
            s_mainWindow = new MainWidget();
        }
    }
    return s_mainWindow;
}

void Application::quit()
{

}
