/****************************************************************************
** Meta object code from reading C++ file 'floatframe.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.15)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../CardWidget/floatframe.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'floatframe.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.15. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FloatFrame_t {
    QByteArrayData data[24];
    char stringdata0[243];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FloatFrame_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FloatFrame_t qt_meta_stringdata_FloatFrame = {
    {
QT_MOC_LITERAL(0, 0, 10), // "FloatFrame"
QT_MOC_LITERAL(1, 11, 12), // "mouseClicked"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 9), // "addWidget"
QT_MOC_LITERAL(4, 35, 8), // "QWidget*"
QT_MOC_LITERAL(5, 44, 1), // "w"
QT_MOC_LITERAL(6, 46, 12), // "cancelWidget"
QT_MOC_LITERAL(7, 59, 15), // "setHideInterval"
QT_MOC_LITERAL(8, 75, 4), // "msec"
QT_MOC_LITERAL(9, 80, 17), // "setExpireInterval"
QT_MOC_LITERAL(10, 98, 11), // "set_enabled"
QT_MOC_LITERAL(11, 110, 7), // "enabled"
QT_MOC_LITERAL(12, 118, 10), // "enterEvent"
QT_MOC_LITERAL(13, 129, 7), // "QEvent*"
QT_MOC_LITERAL(14, 137, 5), // "event"
QT_MOC_LITERAL(15, 143, 10), // "leaveEvent"
QT_MOC_LITERAL(16, 154, 15), // "mousePressEvent"
QT_MOC_LITERAL(17, 170, 12), // "QMouseEvent*"
QT_MOC_LITERAL(18, 183, 17), // "mouseReleaseEvent"
QT_MOC_LITERAL(19, 201, 10), // "timerEvent"
QT_MOC_LITERAL(20, 212, 12), // "QTimerEvent*"
QT_MOC_LITERAL(21, 225, 1), // "e"
QT_MOC_LITERAL(22, 227, 7), // "hideAll"
QT_MOC_LITERAL(23, 235, 7) // "showAll"

    },
    "FloatFrame\0mouseClicked\0\0addWidget\0"
    "QWidget*\0w\0cancelWidget\0setHideInterval\0"
    "msec\0setExpireInterval\0set_enabled\0"
    "enabled\0enterEvent\0QEvent*\0event\0"
    "leaveEvent\0mousePressEvent\0QMouseEvent*\0"
    "mouseReleaseEvent\0timerEvent\0QTimerEvent*\0"
    "e\0hideAll\0showAll"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FloatFrame[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   80,    2, 0x0a /* Public */,
       6,    1,   83,    2, 0x0a /* Public */,
       7,    1,   86,    2, 0x0a /* Public */,
       9,    1,   89,    2, 0x0a /* Public */,
      10,    1,   92,    2, 0x0a /* Public */,
      12,    1,   95,    2, 0x09 /* Protected */,
      15,    1,   98,    2, 0x09 /* Protected */,
      16,    1,  101,    2, 0x09 /* Protected */,
      18,    1,  104,    2, 0x09 /* Protected */,
      19,    1,  107,    2, 0x09 /* Protected */,
      22,    0,  110,    2, 0x09 /* Protected */,
      23,    0,  111,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 17,   14,
    QMetaType::Void, 0x80000000 | 17,   14,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FloatFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FloatFrame *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->mouseClicked(); break;
        case 1: _t->addWidget((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 2: _t->cancelWidget((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 3: _t->setHideInterval((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->setExpireInterval((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->set_enabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->enterEvent((*reinterpret_cast< QEvent*(*)>(_a[1]))); break;
        case 7: _t->leaveEvent((*reinterpret_cast< QEvent*(*)>(_a[1]))); break;
        case 8: _t->mousePressEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 9: _t->mouseReleaseEvent((*reinterpret_cast< QMouseEvent*(*)>(_a[1]))); break;
        case 10: _t->timerEvent((*reinterpret_cast< QTimerEvent*(*)>(_a[1]))); break;
        case 11: _t->hideAll(); break;
        case 12: _t->showAll(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FloatFrame::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FloatFrame::mouseClicked)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FloatFrame::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_FloatFrame.data,
    qt_meta_data_FloatFrame,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FloatFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FloatFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FloatFrame.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int FloatFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void FloatFrame::mouseClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
