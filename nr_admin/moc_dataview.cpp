/****************************************************************************
** DataViewer meta object code from reading C++ file 'dataview.hxx'
**
** Created: ?? 3. ??? 20:01:02 2004
**      by: The Qt MOC ($Id: moc_dataview.cpp,v 1.1 2009/09/08 19:07:40 leo Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "dataview.hxx"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *DataViewer::className() const
{
    return "DataViewer";
}

QMetaObject *DataViewer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_DataViewer( "DataViewer", &DataViewer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString DataViewer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DataViewer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString DataViewer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "DataViewer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* DataViewer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QCanvasView::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "zoom_value", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"set_zoom_slot", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "v", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"show_vert_grid_slot", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "v", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"show_hor_grid_slot", 1, param_slot_2 };
    static const QUMethod slot_3 = {"change_grid_font_slot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "set_zoom_slot(int)", &slot_0, QMetaData::Public },
	{ "show_vert_grid_slot(bool)", &slot_1, QMetaData::Public },
	{ "show_hor_grid_slot(bool)", &slot_2, QMetaData::Public },
	{ "change_grid_font_slot()", &slot_3, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"wheel_up_sig", 0, 0 };
    static const QUMethod signal_1 = {"wheel_down_sig", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "wheel_up_sig()", &signal_0, QMetaData::Public },
	{ "wheel_down_sig()", &signal_1, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"DataViewer", parentObject,
	slot_tbl, 4,
	signal_tbl, 2,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_DataViewer.setMetaObject( metaObj );
    return metaObj;
}

void* DataViewer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "DataViewer" ) )
	return this;
    return QCanvasView::qt_cast( clname );
}

// SIGNAL wheel_up_sig
void DataViewer::wheel_up_sig()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

// SIGNAL wheel_down_sig
void DataViewer::wheel_down_sig()
{
    activate_signal( staticMetaObject()->signalOffset() + 1 );
}

bool DataViewer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: set_zoom_slot((int)static_QUType_int.get(_o+1)); break;
    case 1: show_vert_grid_slot((bool)static_QUType_bool.get(_o+1)); break;
    case 2: show_hor_grid_slot((bool)static_QUType_bool.get(_o+1)); break;
    case 3: change_grid_font_slot(); break;
    default:
	return QCanvasView::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool DataViewer::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: wheel_up_sig(); break;
    case 1: wheel_down_sig(); break;
    default:
	return QCanvasView::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool DataViewer::qt_property( int id, int f, QVariant* v)
{
    return QCanvasView::qt_property( id, f, v);
}

bool DataViewer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
