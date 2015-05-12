/****************************************************************************
** Meta object code from reading C++ file 'meshviewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../meshviewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'meshviewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MeshViewer_t {
    QByteArrayData data[7];
    char stringdata[62];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MeshViewer_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MeshViewer_t qt_meta_stringdata_MeshViewer = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MeshViewer"
QT_MOC_LITERAL(1, 11, 28), // "updateMeshColorByGeoDistance"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 4), // "vidx"
QT_MOC_LITERAL(4, 46, 4), // "lev0"
QT_MOC_LITERAL(5, 51, 4), // "lev1"
QT_MOC_LITERAL(6, 56, 5) // "alpha"

    },
    "MeshViewer\0updateMeshColorByGeoDistance\0"
    "\0vidx\0lev0\0lev1\0alpha"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MeshViewer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       1,    4,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Double,    3,    4,    5,    6,

       0        // eod
};

void MeshViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MeshViewer *_t = static_cast<MeshViewer *>(_o);
        switch (_id) {
        case 0: _t->updateMeshColorByGeoDistance((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->updateMeshColorByGeoDistance((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< double(*)>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MeshViewer::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MeshViewer::updateMeshColorByGeoDistance)) {
                *result = 0;
            }
        }
        {
            typedef void (MeshViewer::*_t)(int , int , int , double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MeshViewer::updateMeshColorByGeoDistance)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject MeshViewer::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_MeshViewer.data,
      qt_meta_data_MeshViewer,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *MeshViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MeshViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_MeshViewer.stringdata))
        return static_cast<void*>(const_cast< MeshViewer*>(this));
    if (!strcmp(_clname, "QGLFunctions"))
        return static_cast< QGLFunctions*>(const_cast< MeshViewer*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int MeshViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MeshViewer::updateMeshColorByGeoDistance(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MeshViewer::updateMeshColorByGeoDistance(int _t1, int _t2, int _t3, double _t4)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
