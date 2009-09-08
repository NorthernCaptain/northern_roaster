#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>

#include <ace/OS_String.h>

#include "debug.hxx"
#include "NameService.h"
#include "StringUtils.h"

NameService::NameService( CORBA::ORB_ptr OrbPtr )
{
    Orb = CORBA::ORB::_duplicate( OrbPtr );
}

/**
 * Инициализация начального контекста имен {@link #rootContext}.
 * Инициализация происходит через попытку разрешения имени, заданного
 * через параметр URI.
 * Если URI не задан, то инициализация происходит через
 * попытку разрешения имени "NameService", то есть предполагается,
 * что конкретный путь был указан через параметр
 * "-ORBInitRef" командной строки.
 *
 *  @param URI полное имя начального контекста имен
 */
void NameService::Bootstrap( const char* URI )
{
    CORBA::Object_var NSref = URI ? Orb->string_to_object( URI ) :
				    Orb->resolve_initial_references( "NameService" );

    rootContext = CosNaming::NamingContext::_narrow( NSref.in() );
    currContext = CosNaming::NamingContext::_duplicate( rootContext.in() );
}

/**
 * Пытается найти существующий или создать новый контекст имен.
 * Начинает создавать контекст только если не смог его найти. Имя контекста
 * можно задавать полным путем в формате, приемлемом {@link #String2Name}.
 * Поиск начинается с {@link #rootContext}, то есть перед использованием
 * этого метода должен быть вызван один из {@link #Bootsrtap}.
 *
 * @param Name_ptr полное имя контекста имен. Если имя пустая строка, то возвращается
 *                 rootContext.
 *
 * @exception ServerException если что-то не так с именем или его компонентами,
 * а также если кто-то в это же время пытается строить такую же структуру
 */
const CosNaming::NamingContext_ptr NameService::GetContext( const char* Name_ptr, bool createNew ) const
    throw ( NameServiceException )
{
    if( !Name_ptr || *Name_ptr == '\0' )
	return CosNaming::NamingContext::_duplicate( currContext.in() );

    CosNaming::NamingContext_var cntxt = ( Name_ptr[0] == '/' ?
		CosNaming::NamingContext::_duplicate( rootContext.in() ):
		CosNaming::NamingContext::_duplicate( currContext.in() ) );

    DBG(9, "Name=" << Name_ptr );

    CosNaming::Name_var name( String2Name( Name_ptr ) );

    for( size_t counter = 0; counter < name->length(); counter++ )
    {
	CosNaming::Name currName( 1 );
	currName.length( 1 );

	currName[ 0 ].id = CORBA::string_dup( name[ counter ].id );
	currName[ 0 ].kind = CORBA::string_dup( name[ counter ].kind );

	try
	{
	    try
	    {
		CORBA::Object_var obj = cntxt->resolve( currName );

		if ( CORBA::is_nil ( obj.in() ) )
		    ERR( "IS_NIL" );

		try
		{
		    cntxt = CosNaming::NamingContext::_narrow( obj.in() );
		}
		catch ( const CORBA::Exception& e )
		{
		    ERR( "CORBA Exception in NS internal: " << e._info() );

		    throw NameServiceException( 0,
						"NameService module: \"ObjectNotExist\" exception caught." );
		}

	    }
	    catch ( const CosNaming::NamingContext::NotFound& )
	    {
		if ( createNew )
		{
		    cntxt = cntxt->bind_new_context( currName );
		    DBG( 9, "Bind new context." );
		}
		else
		    throw;
	    }
	    catch( const CORBA::Exception& e )
	    {
		ERR( "CORBA Exception in NS: " << e._info() );
		throw;
	    }

	}
	catch ( const CosNaming::NamingContext::AlreadyBound& )
	{
	    //Кто-то в параллель строит в NameService такую же структуру имен
	    throw NameServiceException( 0,
					"NameService module: \"AlreadyBound\" exception caught.",
					"Somebody build same structure of names at this time." );
	}
	catch ( const CosNaming::NamingContext::NotFound& nf )
	{
	    std::string tm = "Unresolvable rest of name: ";
	    tm += Name2String( nf.rest_of_name );
	    throw NameServiceException( nf.why,
					"NameService module: \"NotFound\" exception caught.",
					tm.c_str() );
	}
	catch ( const CosNaming::NamingContext::CannotProceed& cnp )
	{
	    std::string tm = "Unresolvable rest of name: ";
	    tm += Name2String( cnp.rest_of_name );
	    throw NameServiceException( 0,
					"NameService module: \"CannotProceed\" exception caught.",
					tm.c_str() );
	}
	catch ( const CosNaming::NamingContext::InvalidName& )
	{
	    throw NameServiceException( 0,
					"NameService module: \"InvalidName\" exception caught.",
					"Zero-length name or invalid symbols in name." );
	}
	catch ( std::exception& se )
	{
	    ERR( "std::exception: " << se.what() );
	    throw;
	}
 	catch ( ... )
 	{
 	    ERR( "unknown exception" );
 	}
    }

    return cntxt._retn();

#if 0
    CosNaming::NamingContext_ptr result;

    try
    {
	result = cntxt._retn();
	return result;
    }
    catch( std::exception& se )
    {
	std::cerr << "exception: '" << se.what() << "'" << std::endl;
	throw;
    }
#endif
}

std::vector< std::string > NameService::GetBindingNames( const CosNaming::NamingContext_ptr& NContext )
{
    std::vector< std::string > result;

    CosNaming::BindingList_var lst;
    CosNaming::BindingIterator_var iter;

    NContext->list( 1000, lst.out(), iter.out() );

    size_t curr = 0, sz = lst->length();

    while( curr != sz )
    {
	CosNaming::Binding bind = lst[ curr ];
	CosNaming::Name cName = bind.binding_name;

	if( cName.length() != 0 )
	    result.push_back( std::string( cName[0].id ) );

	curr++;
    }

    if( !CORBA::is_nil( iter.in() ) )
	iter->destroy();

    return result;
}

static void internal_rl( std::vector< std::string >& ls,
			 const std::string& prefix,
			 CosNaming::NamingContext_ptr nc,
			 bool show_kind,
			 int max_level );

static void internal_sh( std::vector< std::string >& ls,
			 const std::string& prefix,
			 CosNaming::NamingContext_ptr nc,
			 const CosNaming::BindingList& bl,
			 bool show_kind,
			 int max_level )
{
    for ( CORBA::ULong i = 0; i < bl.length (); i++)
    {
	std::string new_node = prefix;

	new_node += bl[i].binding_name[0].id;

	if ( show_kind )
	{
	    new_node += '.';
	    new_node +=  bl[i].binding_name[0].kind;
	}

	if ( bl[i].binding_type == CosNaming::ncontext &&  max_level > 0)
	{
	    CosNaming::Name Name;
	    Name.length (1);
	    Name[0].id   = CORBA::string_dup( bl[i].binding_name[0].id );
	    Name[0].kind = CORBA::string_dup( bl[i].binding_name[0].kind );

	    CORBA::Object_var obj = nc->resolve( Name );

	    CosNaming::NamingContext_var xc =
		CosNaming::NamingContext::_narrow( obj.in() );

	    new_node += '/';

	    internal_rl( ls, new_node, xc.in() , show_kind, max_level -1 );
	}
	else
	{
	    DBG(9, "adding: " << new_node );
	    ls.push_back( new_node );
	}
    }
}

static void internal_rl( std::vector< std::string >& ls,
			 const std::string& prefix,
			 CosNaming::NamingContext_ptr nc,
			 bool show_kind,
			 int max_level )
{
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    nc->list( CHUNK, bl, it );

    internal_sh( ls, prefix, nc, bl.in(), show_kind, max_level );

    if ( !CORBA::is_nil ( it.in () ))
    {
	CORBA::Boolean more;

	do
	{
	    more = it->next_n( CHUNK, bl );
	    internal_sh ( ls, prefix, nc, bl.in(), show_kind, max_level );
	}
	while (more);

	it->destroy ();
    }
}

std::vector< std::string > NameService::RecursiveList( bool show_kind, int max_level )
{
    std::vector< std::string > result;
    std::string prefix;

    internal_rl( result, prefix, currContext.in(), show_kind, max_level );

    return result;
}

/**
   Преобразует строковое представление имени в структуру CosNaming::Name.
   Входные имена вида "/WMS.root/projectName.proj/envName.env/LoginManager.obj".

   @param NameString полное имя конткста имен
*/
CosNaming::Name* NameService::String2Name( const std::string& NameString )
{
    std::vector< std::string > nameComponents =
        StringUtils::makeListFromString( NameString, "/" );

    size_t dotPos = 0, counter = 0;
    std::string tmp = "";
    std::vector< std::string >::iterator i;

    CosNaming::Name_var result = new CosNaming::Name( nameComponents.size() );

    result->length( nameComponents.size() );

    i = nameComponents.begin();

    while( i != nameComponents.end() )
    {
	tmp = *i;
	dotPos = tmp.find( "." );

	if( dotPos == tmp.npos )
	{
	    result[ counter ].id = CORBA::string_dup( tmp.c_str() );
	    result[ counter ].kind = CORBA::string_dup( "" );
	}
	else
	{
	    result[ counter ].id = CORBA::string_dup( tmp.substr(0, dotPos).c_str() );

	    if( (dotPos + 1) != tmp.length() )
		result[ counter ].kind = CORBA::string_dup( tmp.substr(dotPos + 1, tmp.length()).c_str() );
	    else
		result[ counter ].kind = CORBA::string_dup( "" );
	}

	counter++;
	i++;
    }

    return result._retn();
}

/**
   Преобразует структуру CosNaming::Name в строку.
*/
std::string NameService::Name2String( const CosNaming::Name& NameStruct )
{
    size_t nameLen = NameStruct.length(), currElem = 0;
    std::string result = "", tmp = "";


    while( currElem < nameLen )
    {
	result += "/";

	result += ( const char* )NameStruct[ currElem ].id;
	tmp = ( const char* )NameStruct[ currElem ].kind;

	if( tmp != "" )
	    result += "." + tmp;

	currElem++;
    }

    return result;
}

void NameService::Bind( CORBA::Object_ptr Obj_ptr, const char* Name_ptr )
{
    CosNaming::Name_var name = String2Name( Name_ptr );

    currContext->bind( name, Obj_ptr );
}

void NameService::Rebind( CORBA::Object_ptr Obj_ptr, const char* Name_ptr )
{
    CosNaming::Name_var name = String2Name( Name_ptr );

    currContext->rebind( name, Obj_ptr );
}

void NameService::Unbind(const char *Name_ptr)
{
    CosNaming::Name_var name = String2Name( Name_ptr );

    currContext->unbind( name );
}

void  NameService::Destroy( void )
{
    if ( !rootContext->_is_equivalent( currContext.in() ))
    {
	DBG( 9, "root != curr" );
	currContext->destroy();
	currContext = CosNaming::NamingContext::_duplicate( rootContext.in() );
    }
    else
	ERR( "Won't to destroy root context" );
}

std::ostream& operator<< ( std::ostream& s, const NameServiceException& nse )
{
   return s << "NameServiceException caught:\n"
	<< std::setw(16) << "ErrCode: "       << nse.ErrCode
	<< std::setw(16) << "ServerMessage: " << nse.ServerMessage
	<< std::setw(16) << "ToolMessage: "   << nse.ToolMessage << std::endl;
}
