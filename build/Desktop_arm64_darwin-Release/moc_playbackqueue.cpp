/****************************************************************************
** Meta object code from reading C++ file 'playbackqueue.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../audioplayer/playbackqueue.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'playbackqueue.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlaybackQueue_t {
    QByteArrayData data[27];
    char stringdata0[314];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlaybackQueue_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlaybackQueue_t qt_meta_stringdata_PlaybackQueue = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PlaybackQueue"
QT_MOC_LITERAL(1, 14, 19), // "currentIndexChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 8), // "position"
QT_MOC_LITERAL(4, 44, 19), // "currentTrackChanged"
QT_MOC_LITERAL(5, 64, 5), // "Track"
QT_MOC_LITERAL(6, 70, 3), // "trk"
QT_MOC_LITERAL(7, 74, 10), // "loadFailed"
QT_MOC_LITERAL(8, 85, 6), // "loaded"
QT_MOC_LITERAL(9, 92, 12), // "trackChanged"
QT_MOC_LITERAL(10, 105, 5), // "start"
QT_MOC_LITERAL(11, 111, 3), // "end"
QT_MOC_LITERAL(12, 115, 13), // "trackInserted"
QT_MOC_LITERAL(13, 129, 12), // "trackRemoved"
QT_MOC_LITERAL(14, 142, 19), // "playbackModeChanged"
QT_MOC_LITERAL(15, 162, 12), // "PlaybackMode"
QT_MOC_LITERAL(16, 175, 4), // "mode"
QT_MOC_LITERAL(17, 180, 17), // "trackCountChanged"
QT_MOC_LITERAL(18, 198, 5), // "count"
QT_MOC_LITERAL(19, 204, 4), // "next"
QT_MOC_LITERAL(20, 209, 8), // "previous"
QT_MOC_LITERAL(21, 218, 15), // "setCurrentIndex"
QT_MOC_LITERAL(22, 234, 17), // "playqueuePosition"
QT_MOC_LITERAL(23, 252, 7), // "shuffle"
QT_MOC_LITERAL(24, 260, 12), // "playbackMode"
QT_MOC_LITERAL(25, 273, 27), // "PlaybackQueue::PlaybackMode"
QT_MOC_LITERAL(26, 301, 12) // "currentIndex"

    },
    "PlaybackQueue\0currentIndexChanged\0\0"
    "position\0currentTrackChanged\0Track\0"
    "trk\0loadFailed\0loaded\0trackChanged\0"
    "start\0end\0trackInserted\0trackRemoved\0"
    "playbackModeChanged\0PlaybackMode\0mode\0"
    "trackCountChanged\0count\0next\0previous\0"
    "setCurrentIndex\0playqueuePosition\0"
    "shuffle\0playbackMode\0PlaybackQueue::PlaybackMode\0"
    "currentIndex"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlaybackQueue[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       2,  114, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   79,    2, 0x06 /* Public */,
       4,    1,   82,    2, 0x06 /* Public */,
       7,    0,   85,    2, 0x06 /* Public */,
       8,    0,   86,    2, 0x06 /* Public */,
       9,    2,   87,    2, 0x06 /* Public */,
      12,    2,   92,    2, 0x06 /* Public */,
      13,    2,   97,    2, 0x06 /* Public */,
      14,    1,  102,    2, 0x06 /* Public */,
      17,    1,  105,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      19,    0,  108,    2, 0x0a /* Public */,
      20,    0,  109,    2, 0x0a /* Public */,
      21,    1,  110,    2, 0x0a /* Public */,
      23,    0,  113,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, QMetaType::Int,   18,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void,

 // properties: name, type, flags
      24, 0x80000000 | 25, 0x0049510b,
      26, QMetaType::Int, 0x00495103,

 // properties: notify_signal_id
       7,
       0,

       0        // eod
};

void PlaybackQueue::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlaybackQueue *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->currentTrackChanged((*reinterpret_cast< const Track(*)>(_a[1]))); break;
        case 2: _t->loadFailed(); break;
        case 3: _t->loaded(); break;
        case 4: _t->trackChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->trackInserted((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 6: _t->trackRemoved((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->playbackModeChanged((*reinterpret_cast< PlaybackMode(*)>(_a[1]))); break;
        case 8: _t->trackCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->next(); break;
        case 10: _t->previous(); break;
        case 11: _t->setCurrentIndex((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->shuffle(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlaybackQueue::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::currentIndexChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(const Track & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::currentTrackChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::loadFailed)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::loaded)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::trackChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::trackInserted)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::trackRemoved)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(PlaybackMode );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::playbackModeChanged)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PlaybackQueue::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlaybackQueue::trackCountChanged)) {
                *result = 8;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< PlaybackQueue::PlaybackMode >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PlaybackQueue *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< PlaybackQueue::PlaybackMode*>(_v) = _t->playbackMode(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->currentIndex(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<PlaybackQueue *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setPlaybackMode(*reinterpret_cast< PlaybackQueue::PlaybackMode*>(_v)); break;
        case 1: _t->setCurrentIndex(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject PlaybackQueue::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_meta_stringdata_PlaybackQueue.data,
    qt_meta_data_PlaybackQueue,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlaybackQueue::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlaybackQueue::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlaybackQueue.stringdata0))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int PlaybackQueue::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void PlaybackQueue::currentIndexChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PlaybackQueue::currentTrackChanged(const Track & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PlaybackQueue::loadFailed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PlaybackQueue::loaded()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PlaybackQueue::trackChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void PlaybackQueue::trackInserted(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PlaybackQueue::trackRemoved(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PlaybackQueue::playbackModeChanged(PlaybackMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void PlaybackQueue::trackCountChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
