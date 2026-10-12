project(qtsingleapplication)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

add_library(qtsingleapplication  STATIC
    qtsingleapplication.cpp
    qtsingleapplication.h
    qtlocalpeer.cpp
    qtlocalpeer.h
#    qtlockedfile_unix.cpp
#    qtlockedfile_win.cpp
    qtlockedfile.cpp
    qtlockedfile.h
)

target_link_libraries(qtsingleapplication PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

target_compile_definitions(qtsingleapplication PRIVATE QTSINGLEAPPLICATION_LIBRARY)
