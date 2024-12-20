#ifndef APPLICATION_H
#define APPLICATION_H
#include "servermanagewidget.h"
#include <QApplication>

class MainWidget;
class QMenu;

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);
    ~Application();
    static Application *instance();

    void setLanguage(const QString &language = "");

    void setTheme(const QString &theme);

    QMenu *systemMenu();

    void quit();

    static void setFormInCenter(QWidget *frm);

    static int screenWidth();

public slots:

    void showMainWindow();
    void showSystemPreferences();
    void showSeverManager();

private slots:
    MainWidget *mianWindow();

private:
    static MainWidget *s_mainWindow;
    static ServerManageWidget *s_serverManager;

    QByteArray m_lastSession;

    QMenu *m_systemMenu;

    void initTrayMenu();
};

#endif // APPLICATION_H
