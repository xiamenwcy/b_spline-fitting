/****************************************************************************
** Meta object code from reading C++ file 'FitSubWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../FitSubWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FitSubWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_FitSubWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      32,   29,   13,   13, 0x08,
      62,   59,   13,   13, 0x08,
      93,   59,   13,   13, 0x08,
     123,  120,   13,   13, 0x08,
     147,  144,   13,   13, 0x08,
     181,  178,   13,   13, 0x08,
     209,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_FitSubWindow[] = {
    "FitSubWindow\0\0window_close()\0fv\0"
    "set_fitted_mesh_view(bool)\0av\0"
    "set_adjustpoints_enabled(bool)\0"
    "set_adjusted_enabled(bool)\0ev\0"
    "set_error_show(bool)\0mv\0"
    "set_max_error_point_show(bool)\0cv\0"
    "set_controledges_show(bool)\0"
    "updateStatusBar()\0"
};

void FitSubWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FitSubWindow *_t = static_cast<FitSubWindow *>(_o);
        switch (_id) {
        case 0: _t->window_close(); break;
        case 1: _t->set_fitted_mesh_view((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->set_adjustpoints_enabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->set_adjusted_enabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->set_error_show((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->set_max_error_point_show((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->set_controledges_show((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->updateStatusBar(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FitSubWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FitSubWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_FitSubWindow,
      qt_meta_data_FitSubWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FitSubWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FitSubWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FitSubWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FitSubWindow))
        return static_cast<void*>(const_cast< FitSubWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int FitSubWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void FitSubWindow::window_close()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
