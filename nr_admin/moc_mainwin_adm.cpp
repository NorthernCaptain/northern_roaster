/****************************************************************************
** NRMainWinAdm meta object code from reading C++ file 'mainwin_adm.hxx'
**
** Created: ?? 3. ??? 17:40:07 2004
**      by: The Qt MOC ($Id: moc_mainwin_adm.cpp,v 1.1 2009/09/08 19:07:40 leo Exp $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "mainwin_adm.hxx"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.2.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *NRMainWinAdm::className() const
{
    return "NRMainWinAdm";
}

QMetaObject *NRMainWinAdm::metaObj = 0;
static QMetaObjectCleanUp cleanUp_NRMainWinAdm( "NRMainWinAdm", &NRMainWinAdm::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString NRMainWinAdm::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "NRMainWinAdm", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString NRMainWinAdm::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "NRMainWinAdm", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* NRMainWinAdm::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"zoom_in_slot", 0, 0 };
    static const QUMethod slot_1 = {"zoom_out_slot", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "a", &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_2 = {"set_vert_grid_lines_slot", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "a", &static_QUType_ptr, "QAction", QUParameter::In }
    };
    static const QUMethod slot_3 = {"set_hor_grid_lines_slot", 1, param_slot_3 };
    static const QUMethod slot_4 = {"new_view_slot", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "zoom_in_slot()", &slot_0, QMetaData::Protected },
	{ "zoom_out_slot()", &slot_1, QMetaData::Protected },
	{ "set_vert_grid_lines_slot(QAction*)", &slot_2, QMetaData::Protected },
	{ "set_hor_grid_lines_slot(QAction*)", &slot_3, QMetaData::Protected },
	{ "new_view_slot()", &slot_4, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"NRMainWinAdm", parentObject,
	slot_tbl, 5,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_NRMainWinAdm.setMetaObject( metaObj );
    return metaObj;
}

void* NRMainWinAdm::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "NRMainWinAdm" ) )
	return this;
    return QMainWindow::qt_cast( clname );
}

bool NRMainWinAdm::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: zoom_in_slot(); break;
    case 1: zoom_out_slot(); break;
    case 2: set_vert_grid_lines_slot((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 3: set_hor_grid_lines_slot((QAction*)static_QUType_ptr.get(_o+1)); break;
    case 4: new_view_slot(); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool NRMainWinAdm::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool NRMainWinAdm::qt_property( int id, int f, QVariant* v)
{
    return QMainWindow::qt_property( id, f, v);
}

bool NRMainWinAdm::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
