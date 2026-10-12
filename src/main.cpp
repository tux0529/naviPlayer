#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QDebug>

#include "qtsingleapplication/qtsingleapplication.h"

#include "mainwindow.h"
#include "core/appcontext.h"

int main(int argc, char *argv[]) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QCoreApplication::setOrganizationName("cn.redtux");
    QCoreApplication::setOrganizationDomain("redtux.cn");
    QCoreApplication::setApplicationName("naviPlayer");

    QtSingleApplication a(argc, argv);

    if (a.isRunning()) {
        a.sendMessage(a.arguments().join("$"), 3000);
        return EXIT_SUCCESS;
    }

    // 初始化核心服务（数据库 / 设置 / API / 缓存 / 播放器 / 歌词）
    if (!AppContext::instance()->init()) {
        qWarning() << "核心服务初始化失败，应用仍将启动（部分功能不可用）";
    }

    QTranslator translator;
    const QString localeName = QLocale::system().name();
    if (translator.load(QStringLiteral(":/i18n/naviPlayer_%1").arg(localeName)))
        a.installTranslator(&translator);

    MainWindow w;
    w.show();

    const int result = a.exec();
    AppContext::instance()->shutdown();
    return result;
}
