/****************************************************************************
** Meta object code from reading C++ file 'KnotSubWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../KnotSubWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'KnotSubWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_KnotSubWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   30,   14,   14, 0x0a,
      53,   50,   14,   14, 0x0a,
      81,   78,   14,   14, 0x0a,
     104,  101,   14,   14, 0x0a,
     131,  101,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_KnotSubWindow[] = {
    "KnotSubWindow\0\0window_close()\0kv\0"
    "knots_view(bool)\0cv\0set_curvature_show(bool)\0"
    "dv\0set_mesh_view(bool)\0ev\0"
    "error_curvature_view(bool)\0"
    "error_fitting_view(bool)\0"
};

void KnotSubWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        KnotSubWindow *_t = static_cast<KnotSubWindow *>(_o);
        switch (_id) {
        case 0: _t->window_close(); break;
        case 1: _t->knots_view((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->set_curvature_show((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->set_mesh_view((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->error_curvature_view((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->error_fitting_view((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData KnotSubWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject KnotSubWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_KnotSubWindow,
      qt_meta_data_KnotSubWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &KnotSubWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *KnotSubWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *KnotSubWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_KnotSubWindow))
        return static_cast<void*>(const_cast< KnotSubWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int KnotSubWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void KnotSubWindow::window_close()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
