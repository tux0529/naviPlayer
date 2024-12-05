/****************************************************************************
** Meta object code from reading C++ file 'playworker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.12)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../audioplayer/playworker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'playworker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.12. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PlayWorker_t {
    QByteArrayData data[22];
    char stringdata0[220];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PlayWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PlayWorker_t qt_meta_stringdata_PlayWorker = {
    {
QT_MOC_LITERAL(0, 0, 10), // "PlayWorker"
QT_MOC_LITERAL(1, 11, 5), // "ERROR"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 18), // "AudioPlayer::Error"
QT_MOC_LITERAL(4, 37, 5), // "error"
QT_MOC_LITERAL(5, 43, 3), // "msg"
QT_MOC_LITERAL(6, 47, 15), // "durationChanged"
QT_MOC_LITERAL(7, 63, 8), // "duration"
QT_MOC_LITERAL(8, 72, 15), // "positionChanged"
QT_MOC_LITERAL(9, 88, 8), // "position"
QT_MOC_LITERAL(10, 97, 18), // "mediaStatusChanged"
QT_MOC_LITERAL(11, 116, 24), // "AudioPlayer::MediaStatus"
QT_MOC_LITERAL(12, 141, 6), // "status"
QT_MOC_LITERAL(13, 148, 12), // "stateChanged"
QT_MOC_LITERAL(14, 161, 18), // "AudioPlayer::State"
QT_MOC_LITERAL(15, 180, 5), // "state"
QT_MOC_LITERAL(16, 186, 4), // "play"
QT_MOC_LITERAL(17, 191, 8), // "filename"
QT_MOC_LITERAL(18, 200, 4), // "stop"
QT_MOC_LITERAL(19, 205, 5), // "pause"
QT_MOC_LITERAL(20, 211, 4), // "seek"
QT_MOC_LITERAL(21, 216, 3) // "pos"

    },
    "PlayWorker\0ERROR\0\0AudioPlayer::Error\0"
    "error\0msg\0durationChanged\0duration\0"
    "positionChanged\0position\0mediaStatusChanged\0"
    "AudioPlayer::MediaStatus\0status\0"
    "stateChanged\0AudioPlayer::State\0state\0"
    "play\0filename\0stop\0pause\0seek\0pos"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PlayWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   69,    2, 0x06 /* Public */,
       6,    1,   74,    2, 0x06 /* Public */,
       8,    1,   77,    2, 0x06 /* Public */,
      10,    1,   80,    2, 0x06 /* Public */,
      13,    1,   83,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    1,   86,    2, 0x0a /* Public */,
      16,    0,   89,    2, 0x2a /* Public | MethodCloned */,
      18,    0,   90,    2, 0x0a /* Public */,
      19,    0,   91,    2, 0x0a /* Public */,
      20,    1,   92,    2, 0x0a /* Public */,
      20,    0,   95,    2, 0x2a /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    5,
    QMetaType::Void, QMetaType::LongLong,    7,
    QMetaType::Void, QMetaType::LongLong,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   21,
    QMetaType::Void,

       0        // eod
};

void PlayWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PlayWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ERROR((*reinterpret_cast< AudioPlayer::Error(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: _t->durationChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 2: _t->positionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 3: _t->mediaStatusChanged((*reinterpret_cast< AudioPlayer::MediaStatus(*)>(_a[1]))); break;
        case 4: _t->stateChanged((*reinterpret_cast< AudioPlayer::State(*)>(_a[1]))); break;
        case 5: _t->play((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->play(); break;
        case 7: _t->stop(); break;
        case 8: _t->pause(); break;
        case 9: _t->seek((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 10: _t->seek(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PlayWorker::*)(AudioPlayer::Error , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlayWorker::ERROR)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PlayWorker::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlayWorker::durationChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PlayWorker::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlayWorker::positionChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PlayWorker::*)(AudioPlayer::MediaStatus );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlayWorker::mediaStatusChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PlayWorker::*)(AudioPlayer::State );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PlayWorker::stateChanged)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PlayWorker::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_PlayWorker.data,
    qt_meta_data_PlayWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PlayWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlayWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PlayWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PlayWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void PlayWorker::ERROR(AudioPlayer::Error _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PlayWorker::durationChanged(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PlayWorker::positionChanged(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PlayWorker::mediaStatusChanged(AudioPlayer::MediaStatus _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PlayWorker::stateChanged(AudioPlayer::State _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
