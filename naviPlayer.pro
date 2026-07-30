QT       += core gui widgets network sql multimedia

CONFIG   += c++17
CONFIG   -= warn_on
CONFIG   += warn_off

TARGET = naviPlayer
TEMPLATE = app

# Fix for arm64 build issue
QMAKE_CXXFLAGS += -Wno-implicit-function-declaration

# Set deployment target
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 13.0

greaterThan(QT_MAJOR_VERSION, 5) {
    DEFINES += NAVIPLAYER_QT6
} else {
    DEFINES += NAVIPLAYER_QT5
}

# FFmpeg 6.1 configuration
FFMPEG_DIR = $$PWD/libs/ffmpeg61

INCLUDEPATH += \
    $$FFMPEG_DIR/include

LIBS += \
    -L$$FFMPEG_DIR/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswresample

# Copy FFmpeg libraries to app bundle on macOS
macx {
    FFMPEG_LIBS.files = \
        $$FFMPEG_DIR/lib/libavcodec.60.dylib \
        $$FFMPEG_DIR/lib/libavformat.60.dylib \
        $$FFMPEG_DIR/lib/libavutil.58.dylib \
        $$FFMPEG_DIR/lib/libswresample.4.dylib

    FFMPEG_LIBS.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += FFMPEG_LIBS
}

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/subsonicapi.cpp \
    src/databasemanager.cpp \
    src/audioplayer.cpp \
    src/cachemanager.cpp \
    src/settingsdialog.cpp \
    src/ffmpegaudiodecoder.cpp \
    src/models/song.cpp \
    src/models/album.cpp \
    src/models/artist.cpp \
    src/models/playlist.cpp \
    src/widgets/navigationwidget.cpp \
    src/widgets/playercontrolbar.cpp \
    src/widgets/songlistwidget.cpp \
    src/widgets/albumwidget.cpp \
    src/widgets/artistwidget.cpp \
    src/widgets/albumdetailwidget.cpp \
    src/widgets/playlistwidget.cpp \
    src/widgets/searchwidget.cpp \
    src/widgets/favoriteswidget.cpp \
    src/widgets/settingswidget.cpp \
    src/widgets/homewidget.cpp

HEADERS += \
    src/qt_compat.h \
    src/mainwindow.h \
    src/subsonicapi.h \
    src/databasemanager.h \
    src/audioplayer.h \
    src/cachemanager.h \
    src/settingsdialog.h \
    src/ffmpegaudiodecoder.h \
    src/models/song.h \
    src/models/album.h \
    src/models/artist.h \
    src/models/playlist.h \
    src/widgets/navigationwidget.h \
    src/widgets/playercontrolbar.h \
    src/widgets/songlistwidget.h \
    src/widgets/albumwidget.h \
    src/widgets/artistwidget.h \
    src/widgets/albumdetailwidget.h \
    src/widgets/playlistwidget.h \
    src/widgets/searchwidget.h \
    src/widgets/favoriteswidget.h \
    src/widgets/settingswidget.h \
    src/widgets/homewidget.h

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

macx {
    # macOS specific settings
    ICON =
}

win32 {
    # Windows specific settings
}
