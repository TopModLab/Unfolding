/****************************************************************************
** Meta object code from reading C++ file 'colormapeditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../extras/colormap_editor/colormapeditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'colormapeditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ColorPatch_t {
    QByteArrayData data[3];
    char stringdata[24];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ColorPatch_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ColorPatch_t qt_meta_stringdata_ColorPatch = {
    {
QT_MOC_LITERAL(0, 0, 10), // "ColorPatch"
QT_MOC_LITERAL(1, 11, 11), // "sig_clicked"
QT_MOC_LITERAL(2, 23, 0) // ""

    },
    "ColorPatch\0sig_clicked\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ColorPatch[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void ColorPatch::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ColorPatch *_t = static_cast<ColorPatch *>(_o);
        switch (_id) {
        case 0: _t->sig_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ColorPatch::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ColorPatch::sig_clicked)) {
                *result = 0;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ColorPatch::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ColorPatch.data,
      qt_meta_data_ColorPatch,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ColorPatch::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ColorPatch::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ColorPatch.stringdata))
        return static_cast<void*>(const_cast< ColorPatch*>(this));
    return QWidget::qt_metacast(_clname);
}

int ColorPatch::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ColorPatch::sig_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
struct qt_meta_stringdata_ColormapEditor_t {
    QByteArrayData data[10];
    char stringdata[180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ColormapEditor_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ColormapEditor_t qt_meta_stringdata_ColormapEditor = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ColormapEditor"
QT_MOC_LITERAL(1, 15, 12), // "colorChanged"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 29), // "slot_updateNegPatchWithSlider"
QT_MOC_LITERAL(4, 59, 3), // "val"
QT_MOC_LITERAL(5, 63, 27), // "slot_updateNegPatchWithSpin"
QT_MOC_LITERAL(6, 91, 29), // "slot_updatePosPatchWithSlider"
QT_MOC_LITERAL(7, 121, 27), // "slot_updatePosPatchWithSpin"
QT_MOC_LITERAL(8, 149, 13), // "updatePatches"
QT_MOC_LITERAL(9, 163, 16) // "slot_changeColor"

    },
    "ColormapEditor\0colorChanged\0\0"
    "slot_updateNegPatchWithSlider\0val\0"
    "slot_updateNegPatchWithSpin\0"
    "slot_updatePosPatchWithSlider\0"
    "slot_updatePosPatchWithSpin\0updatePatches\0"
    "slot_changeColor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ColormapEditor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   50,    2, 0x08 /* Private */,
       5,    1,   53,    2, 0x08 /* Private */,
       6,    1,   56,    2, 0x08 /* Private */,
       7,    1,   59,    2, 0x08 /* Private */,
       8,    0,   62,    2, 0x08 /* Private */,
       9,    0,   63,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ColormapEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ColormapEditor *_t = static_cast<ColormapEditor *>(_o);
        switch (_id) {
        case 0: _t->colorChanged(); break;
        case 1: _t->slot_updateNegPatchWithSlider((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slot_updateNegPatchWithSpin((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->slot_updatePosPatchWithSlider((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->slot_updatePosPatchWithSpin((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->updatePatches(); break;
        case 6: _t->slot_changeColor(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ColormapEditor::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ColormapEditor::colorChanged)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject ColormapEditor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ColormapEditor.data,
      qt_meta_data_ColormapEditor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ColormapEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ColormapEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ColormapEditor.stringdata))
        return static_cast<void*>(const_cast< ColormapEditor*>(this));
    return QWidget::qt_metacast(_clname);
}

int ColormapEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void ColormapEditor::colorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
