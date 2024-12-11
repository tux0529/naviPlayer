QT       += core gui multimedia network sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += G_DEBUG

INCLUDEPATH     += $$PWD/common
INCLUDEPATH     += $$PWD/pages
INCLUDEPATH     += $$PWD/audioplayer


SOURCES += \
    CardWidget/cardlistwidget.cpp \
    CardWidget/cardwidget.cpp \
    CardWidget/floatframe.cpp \
    application.cpp \
    audioplayer/audioplayer.cpp \
    audioplayer/playbackqueue.cpp \
    audioplayer/playworker.cpp \
    common/albumobject.cpp \
    common/anchoritemdelegate.cpp \
    common/anchortableview.cpp \
    common/artist.cpp \
    common/artistobject.cpp \
    common/config.cpp \
    common/genre.cpp \
    common/mediabase.cpp \
    common/mediahelper.cpp \
    common/playlist.cpp \
    common/album.cpp \
    common/iconhelper.cpp \
#    songstablemodel.cpp \
    common/playlistobject.cpp \
    common/track.cpp \
    common/trackobject.cpp \
    common/tracktablemodel.cpp \
    common/tracktableview.cpp \
    main.cpp \
    mainwidget.cpp \
    pages/albuminfopage.cpp \
    pages/albumlistmodel.cpp \
    pages/albumlistpage.cpp \
    pages/artistspage.cpp \
    pages/favoritespage.cpp \
    pages/genrelistpage.cpp \
    pages/homepage.cpp \
    pages/playlistinfopage.cpp \
    pages/playlistspage.cpp \
    pages/playqueuewidget.cpp \
    servermanagewidget.cpp

HEADERS += \
    CardWidget/cardlistwidget.h \
    CardWidget/cardwidget.h \
    CardWidget/floatframe.h \
    application.h \
    audioplayer/audioplayer.h \
    audioplayer/playbackqueue.h \
    audioplayer/playworker.h \
    common/albumobject.h \
    common/anchoritemdelegate.h \
    common/anchortableview.h \
    common/artist.h \
    common/artistobject.h \
    common/config.h \
    common/genre.h \
    common/mediabase.h \
    common/mediahelper.h \
    common/playlist.h \
    common/album.h \
    common/iconhelper.h \
#    songstablemodel.h \
    common/playlistobject.h \
    common/track.h \
    common/trackobject.h \
    common/tracktablemodel.h \
    common/tracktableview.h \
    mainwidget.h \
    pages/albuminfopage.h \
    pages/albumlistmodel.h \
    pages/albumlistpage.h \
    pages/artistspage.h \
    pages/favoritespage.h \
    pages/genrelistpage.h \
    pages/homepage.h \
    pages/playlistinfopage.h \
    pages/playlistspage.h \
    pages/playqueuewidget.h \
    servermanagewidget.h

FORMS += \
    CardWidget/cardlistwidget.ui \
    CardWidget/cardwidget.ui \
    mainwidget.ui \
    pages/albuminfopage.ui \
    pages/albumlistpage.ui \
    pages/artistspage.ui \
    pages/favoritespage.ui \
    pages/genrelistpage.ui \
    pages/homepage.ui \
    pages/playlistinfopage.ui \
    pages/playlistspage.ui \
    pages/playqueuewidget.ui \
    servermanagewidget.ui

RESOURCES += \
    CardWidget/card.qrc \
    resources/main.qrc \
    resources/theme.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#WINDOWS平台
win32 {

    msvc{
        QMAKE_CFLAGS += -source-charset:utf-8
        QMAKE_CXXFLAGS +=  /source-charset:utf-8 /execution-charset:utf-8
    }

    RC_FILE += resources/naviPlayer.rc

    INCLUDEPATH +=  $$PWD/lib/win64/ffmpeg/include

    contains(QT_ARCH, x86_64) {
        message("x64")
        # 依赖模块
        LIBS += \
                -L$$PWD/lib/win64/ffmpeg/lib -lavformat \
                -L$$PWD/lib/win64/ffmpeg/lib -lavcodec \
                 -L$$PWD/lib/win64/ffmpeg/lib -lavutil \
                 -L$$PWD/lib/win64/ffmpeg/lib -lswscale \
                -L$$PWD/lib/win64/ffmpeg/lib -lswresample \


    }
}

unix:!macx {
# linux only
}

macx {

    contains(QT_ARCH, arm64){
        message("macOS_arm64")
        INCLUDEPATH += $$PWD/lib/macOS/ffmpeg61_arm64/include

        LIBS += \
            -L$$PWD/lib/macOS/ffmpeg61_arm64/lib -lavformat -lavcodec -lswresample \
            # -L$$PWD/lib/macOS/ffmpeg61_arm64/lib -lavutil \
            # -L$$PWD/lib/macOS/ffmpeg61_arm64/lib -lswscale \
    }

    contains(QT_ARCH, x86_64) {
        message("macOS_x64")
        INCLUDEPATH += $$PWD/lib/macOS/ffmpeg/include

        LIBS += \
            -L$$PWD/lib/macOS/ffmpeg/lib -lavformat -lavcodec -lswresample \
    }

    ICON = resources/AppIcon.icns
}



# Setting the Application Icon
# https://doc.qt.io/qt-5/appicon.html




