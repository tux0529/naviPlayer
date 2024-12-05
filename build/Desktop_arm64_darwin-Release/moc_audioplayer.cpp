/****************************************************************************
** Meta object code from reading C++ file 'audioplayer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../audioplayer/audioplayer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'audioplayer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_AudioPlayer_t {
    QByteArrayData data[57];
    char stringdata0[665];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_AudioPlayer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_AudioPlayer_t qt_meta_stringdata_AudioPlayer = {
    {
QT_MOC_LITERAL(0, 0, 11), // "AudioPlayer"
QT_MOC_LITERAL(1, 12, 21), // "audioAvailableChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 9), // "available"
QT_MOC_LITERAL(4, 45, 19), // "bufferStatusChanged"
QT_MOC_LITERAL(5, 65, 13), // "percentFilled"
QT_MOC_LITERAL(6, 79, 22), // "customAudioRoleChanged"
QT_MOC_LITERAL(7, 102, 4), // "role"
QT_MOC_LITERAL(8, 107, 15), // "durationChanged"
QT_MOC_LITERAL(9, 123, 8), // "duration"
QT_MOC_LITERAL(10, 132, 5), // "ERROR"
QT_MOC_LITERAL(11, 138, 18), // "AudioPlayer::Error"
QT_MOC_LITERAL(12, 157, 5), // "error"
QT_MOC_LITERAL(13, 163, 18), // "mediaStatusChanged"
QT_MOC_LITERAL(14, 182, 24), // "AudioPlayer::MediaStatus"
QT_MOC_LITERAL(15, 207, 6), // "status"
QT_MOC_LITERAL(16, 214, 12), // "mutedChanged"
QT_MOC_LITERAL(17, 227, 5), // "muted"
QT_MOC_LITERAL(18, 233, 19), // "playbackRateChanged"
QT_MOC_LITERAL(19, 253, 4), // "rate"
QT_MOC_LITERAL(20, 258, 15), // "positionChanged"
QT_MOC_LITERAL(21, 274, 8), // "position"
QT_MOC_LITERAL(22, 283, 15), // "seekableChanged"
QT_MOC_LITERAL(23, 299, 8), // "seekable"
QT_MOC_LITERAL(24, 308, 12), // "stateChanged"
QT_MOC_LITERAL(25, 321, 18), // "AudioPlayer::State"
QT_MOC_LITERAL(26, 340, 5), // "state"
QT_MOC_LITERAL(27, 346, 13), // "volumeChanged"
QT_MOC_LITERAL(28, 360, 6), // "volume"
QT_MOC_LITERAL(29, 367, 12), // "mediaChanged"
QT_MOC_LITERAL(30, 380, 5), // "media"
QT_MOC_LITERAL(31, 386, 14), // "workerPauseSig"
QT_MOC_LITERAL(32, 401, 13), // "workerStopSig"
QT_MOC_LITERAL(33, 415, 12), // "seekPosition"
QT_MOC_LITERAL(34, 428, 3), // "pos"
QT_MOC_LITERAL(35, 432, 5), // "pause"
QT_MOC_LITERAL(36, 438, 4), // "play"
QT_MOC_LITERAL(37, 443, 8), // "filename"
QT_MOC_LITERAL(38, 452, 9), // "playAudio"
QT_MOC_LITERAL(39, 462, 5), // "Track"
QT_MOC_LITERAL(40, 468, 8), // "setMuted"
QT_MOC_LITERAL(41, 477, 11), // "setPosition"
QT_MOC_LITERAL(42, 489, 9), // "setVolume"
QT_MOC_LITERAL(43, 499, 4), // "stop"
QT_MOC_LITERAL(44, 504, 8), // "setAudio"
QT_MOC_LITERAL(45, 513, 3), // "trk"
QT_MOC_LITERAL(46, 517, 16), // "setPlaybackQueue"
QT_MOC_LITERAL(47, 534, 14), // "PlaybackQueue*"
QT_MOC_LITERAL(48, 549, 5), // "queue"
QT_MOC_LITERAL(49, 555, 7), // "onError"
QT_MOC_LITERAL(50, 563, 3), // "msg"
QT_MOC_LITERAL(51, 567, 20), // "onMediaStatusChanged"
QT_MOC_LITERAL(52, 588, 14), // "onStateChanged"
QT_MOC_LITERAL(53, 603, 17), // "onDurationChanged"
QT_MOC_LITERAL(54, 621, 17), // "onPositionChanged"
QT_MOC_LITERAL(55, 639, 19), // "onTrackCountChanged"
QT_MOC_LITERAL(56, 659, 5) // "count"

    },
    "AudioPlayer\0audioAvailableChanged\0\0"
    "available\0bufferStatusChanged\0"
    "percentFilled\0customAudioRoleChanged\0"
    "role\0durationChanged\0duration\0ERROR\0"
    "AudioPlayer::Error\0error\0mediaStatusChanged\0"
    "AudioPlayer::MediaStatus\0status\0"
    "mutedChanged\0muted\0playbackRateChanged\0"
    "rate\0positionChanged\0position\0"
    "seekableChanged\0seekable\0stateChanged\0"
    "AudioPlayer::State\0state\0volumeChanged\0"
    "volume\0mediaChanged\0media\0workerPauseSig\0"
    "workerStopSig\0seekPosition\0pos\0pause\0"
    "play\0filename\0playAudio\0Track\0setMuted\0"
    "setPosition\0setVolume\0stop\0setAudio\0"
    "trk\0setPlaybackQueue\0PlaybackQueue*\0"
    "queue\0onError\0msg\0onMediaStatusChanged\0"
    "onStateChanged\0onDurationChanged\0"
    "onPositionChanged\0onTrackCountChanged\0"
    "count"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_AudioPlayer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      16,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  174,    2, 0x06 /* Public */,
       4,    1,  177,    2, 0x06 /* Public */,
       6,    1,  180,    2, 0x06 /* Public */,
       8,    1,  183,    2, 0x06 /* Public */,
      10,    1,  186,    2, 0x06 /* Public */,
      13,    1,  189,    2, 0x06 /* Public */,
      16,    1,  192,    2, 0x06 /* Public */,
      18,    1,  195,    2, 0x06 /* Public */,
      20,    1,  198,    2, 0x06 /* Public */,
      22,    1,  201,    2, 0x06 /* Public */,
      24,    1,  204,    2, 0x06 /* Public */,
      27,    1,  207,    2, 0x06 /* Public */,
      29,    1,  210,    2, 0x06 /* Public */,
      31,    0,  213,    2, 0x06 /* Public */,
      32,    0,  214,    2, 0x06 /* Public */,
      33,    1,  215,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      35,    0,  218,    2, 0x0a /* Public */,
      36,    1,  219,    2, 0x0a /* Public */,
      36,    0,  222,    2, 0x2a /* Public | MethodCloned */,
      38,    1,  223,    2, 0x0a /* Public */,
      40,    1,  226,    2, 0x0a /* Public */,
      41,    1,  229,    2, 0x0a /* Public */,
      42,    1,  232,    2, 0x0a /* Public */,
      43,    0,  235,    2, 0x0a /* Public */,
      44,    1,  236,    2, 0x0a /* Public */,
      46,    1,  239,    2, 0x0a /* Public */,
      49,    2,  242,    2, 0x0a /* Public */,
      51,    1,  247,    2, 0x0a /* Public */,
      52,    1,  250,    2, 0x0a /* Public */,
      53,    1,  253,    2, 0x08 /* Private */,
      54,    1,  256,    2, 0x08 /* Private */,
      55,    1,  259,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::LongLong,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::QReal,   19,
    QMetaType::Void, QMetaType::LongLong,   21,
    QMetaType::Void, QMetaType::Bool,   23,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void, QMetaType::QString,   30,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::LongLong,   34,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   37,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 39,    2,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void, QMetaType::LongLong,   21,
    QMetaType::Void, QMetaType::Int,   28,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 39,   45,
    QMetaType::Void, 0x80000000 | 47,   48,
    QMetaType::Void, 0x80000000 | 11, QMetaType::QString,   12,   50,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void, QMetaType::LongLong,    9,
    QMetaType::Void, QMetaType::LongLong,   21,
    QMetaType::Void, QMetaType::Int,   56,

       0        // eod
};

void AudioPlayer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AudioPlayer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->audioAvailableChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->bufferStatusChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->customAudioRoleChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->durationChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 4: _t->ERROR((*reinterpret_cast< AudioPlayer::Error(*)>(_a[1]))); break;
        case 5: _t->mediaStatusChanged((*reinterpret_cast< AudioPlayer::MediaStatus(*)>(_a[1]))); break;
        case 6: _t->mutedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->playbackRateChanged((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 8: _t->positionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 9: _t->seekableChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->stateChanged((*reinterpret_cast< AudioPlayer::State(*)>(_a[1]))); break;
        case 11: _t->volumeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->mediaChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->workerPauseSig(); break;
        case 14: _t->workerStopSig(); break;
        case 15: _t->seekPosition((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 16: _t->pause(); break;
        case 17: _t->play((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: _t->play(); break;
        case 19: _t->playAudio((*reinterpret_cast< const Track(*)>(_a[1]))); break;
        case 20: _t->setMuted((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 21: _t->setPosition((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 22: _t->setVolume((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->stop(); break;
        case 24: _t->setAudio((*reinterpret_cast< Track(*)>(_a[1]))); break;
        case 25: _t->setPlaybackQueue((*reinterpret_cast< PlaybackQueue*(*)>(_a[1]))); break;
        case 26: _t->onError((*reinterpret_cast< AudioPlayer::Error(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 27: _t->onMediaStatusChanged((*reinterpret_cast< AudioPlayer::MediaStatus(*)>(_a[1]))); break;
        case 28: _t->onStateChanged((*reinterpret_cast< AudioPlayer::State(*)>(_a[1]))); break;
        case 29: _t->onDurationChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 30: _t->onPositionChanged((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 31: _t->onTrackCountChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AudioPlayer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::audioAvailableChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::bufferStatusChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::customAudioRoleChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::durationChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(AudioPlayer::Error );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::ERROR)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(AudioPlayer::MediaStatus );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::mediaStatusChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::mutedChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(qreal );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::playbackRateChanged)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::positionChanged)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::seekableChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(AudioPlayer::State );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::stateChanged)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::volumeChanged)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::mediaChanged)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::workerPauseSig)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::workerStopSig)) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (AudioPlayer::*)(qint64 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&AudioPlayer::seekPosition)) {
                *result = 15;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject AudioPlayer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_AudioPlayer.data,
    qt_meta_data_AudioPlayer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *AudioPlayer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AudioPlayer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_AudioPlayer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AudioPlayer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 32)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 32)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void AudioPlayer::audioAvailableChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AudioPlayer::bufferStatusChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AudioPlayer::customAudioRoleChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void AudioPlayer::durationChanged(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void AudioPlayer::ERROR(AudioPlayer::Error _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void AudioPlayer::mediaStatusChanged(AudioPlayer::MediaStatus _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void AudioPlayer::mutedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void AudioPlayer::playbackRateChanged(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void AudioPlayer::positionChanged(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void AudioPlayer::seekableChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void AudioPlayer::stateChanged(AudioPlayer::State _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void AudioPlayer::volumeChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void AudioPlayer::mediaChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void AudioPlayer::workerPauseSig()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void AudioPlayer::workerStopSig()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}

// SIGNAL 15
void AudioPlayer::seekPosition(qint64 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
