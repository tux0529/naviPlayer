QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += TEST

INCLUDEPATH     += $$PWD/../common


SOURCES += \
    ../common/mediabase.cpp \
    ../common/track.cpp \
    audioplayer.cpp \
    playbackqueue.cpp \
    playworker.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    ../common/mediabase.h \
    ../common/track.h \
    audioplayer.h \
    playbackqueue.h \
    playworker.h \
    widget.h

FORMS += \
    widget.ui

RESOURCES += \

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
        INCLUDEPATH += $$PWD/../lib/macOS/ffmpeg/include

        LIBS += \
            -L$$PWD/../lib/macOS/ffmpeg/lib -lavformat -lavcodec -lswresample \
    }

}



# Setting the Application Icon
# https://doc.qt.io/qt-5/appicon.html




