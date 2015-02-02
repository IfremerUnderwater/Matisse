/****************************************************************************
** Meta object code from reading C++ file 'RTVideoDirectShowProvider.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/RTVideoDirectShowProvider.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RTVideoDirectShowProvider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RTVideoDirectShowProvider[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x05,
      49,   26,   26,   26, 0x05,

 // slots: signature, parameters, type, tag, flags
      79,   70,   26,   26, 0x0a,
     101,   26,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_RTVideoDirectShowProvider[] = {
    "RTVideoDirectShowProvider\0\0"
    "signal_previewStart()\0signal_previewStop()\0"
    "dim2line\0dim2Received(QString)\0"
    "captureImage()\0"
};

void RTVideoDirectShowProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        RTVideoDirectShowProvider *_t = static_cast<RTVideoDirectShowProvider *>(_o);
        switch (_id) {
        case 0: _t->signal_previewStart(); break;
        case 1: _t->signal_previewStop(); break;
        case 2: _t->dim2Received((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->captureImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData RTVideoDirectShowProvider::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject RTVideoDirectShowProvider::staticMetaObject = {
    { &ImageProvider::staticMetaObject, qt_meta_stringdata_RTVideoDirectShowProvider,
      qt_meta_data_RTVideoDirectShowProvider, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RTVideoDirectShowProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RTVideoDirectShowProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RTVideoDirectShowProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RTVideoDirectShowProvider))
        return static_cast<void*>(const_cast< RTVideoDirectShowProvider*>(this));
    if (!strcmp(_clname, "Chrisar.ImageProvider/1.1"))
        return static_cast< MatisseCommon::ImageProvider*>(const_cast< RTVideoDirectShowProvider*>(this));
    return ImageProvider::qt_metacast(_clname);
}

int RTVideoDirectShowProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ImageProvider::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void RTVideoDirectShowProvider::signal_previewStart()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void RTVideoDirectShowProvider::signal_previewStop()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
