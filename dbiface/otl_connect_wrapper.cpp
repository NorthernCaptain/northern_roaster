#include "dbglog.h"

#include <sstream>

#include <unicode/ucnv.h>

//#define OTL_TRACE_LEVEL 0xff
//#define OTL_TRACE_STREAM *(DL.Out())

#include <otlv4.h>

#include "DBSObjects.h"
#include "otl_connect_wrapper_impl.h"
#include "otl_stream_wrapper_impl.h"

#include "configman.hxx"

static bool initialized = false;

static UConverter* dbConverter = 0;

#ifdef OTL_ODBC
static long int isolationLevel = -1;
#endif

otl_connect_wrapper::~otl_connect_wrapper()
{
}


otl_connect_wrapper_impl::otl_connect_wrapper_impl( const std::string& connectString ) throw( DBQueryException )
    : rawConnect_( 0 )
{
    try
    {
	DBG( 1, "open new DB connection to: '" << connectString << "'" );
	rawConnect_ = new otl_connect( connectString.c_str() );

#ifdef OTL_ODBC
	if( isolationLevel != -1 )
	    rawConnect_->set_transaction_isolation_level( isolationLevel );
#endif
    }
    catch ( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_connect_wrapper_impl() otl_exception caught:",
				mess.str() );
    }
}

otl_connect_wrapper_impl::~otl_connect_wrapper_impl()
{
    if ( rawConnect_ != 0 )
	delete rawConnect_;
}

void otl_connect_wrapper_impl::commit() throw( DBQueryException )
{
    if( !rawConnect_ )
    {
	ERR( "NULL raw otl connect pointer!" );
	return;
    }

    try
    {
	rawConnect_->commit();
    }
    catch ( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_connect_wrapper_impl::commit() otl_exception caught:",
				mess.str() );
    }
}

void otl_connect_wrapper_impl::rollback() throw( DBQueryException )
{
    if( !rawConnect_ )
    {
	ERR( "NULL raw otl connect pointer!" );
	return;
    }

    try
    {
	rawConnect_->rollback();
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_connect_wrapper_impl::rollback() otl_exception caught:",
				mess.str() );
    }
}

void otl_connect_wrapper_impl::set_cursor_type( const CURSOR_TYPE
#ifdef OTL_ODBC
						acursor_type 
#endif
						)
{
    if( !rawConnect_ )
    {
	ERR( "NULL raw otl connect pointer!" );
	return;
    }

#ifdef OTL_ODBC
    switch( acursor_type )
    {
    case WRAP_CURSOR_FORWARD_ONLY:
	rawConnect_->set_cursor_type( SQL_CURSOR_FORWARD_ONLY );
	break;
    case WRAP_CURSOR_STATIC:
	rawConnect_->set_cursor_type( SQL_CURSOR_STATIC );
	break;
    case WRAP_CURSOR_KEYSET_DRIVEN:
	rawConnect_->set_cursor_type( SQL_CURSOR_KEYSET_DRIVEN );
	break;
    case WRAP_CURSOR_DYNAMIC:
	rawConnect_->set_cursor_type( SQL_CURSOR_DYNAMIC );
	break;	
    case WRAP_CURSOR_DEFAULT:
	rawConnect_->set_cursor_type( SQL_CURSOR_TYPE_DEFAULT );
	break;
    }
#endif
}

otl_stream_wrapper* otl_connect_wrapper_impl::new_stream
	( int bufSize, 
	  const std::string& query ) throw ( DBQueryException )
{
    return new otl_stream_wrapper_impl( shared_from_this(),
					bufSize, query );
}

UConverter* otl_connect_wrapper_impl::getConverter( void ) const
{
    return dbConverter;
}

void otl_wrapper_impl::otl_initialize( void )
{
    ACE_TCHAR  buf[128]="";

    if ( initialized )
	return;

    otl_connect::otl_initialize();

    CFGMAN::instance()->get_cstring_value("common", "db_encoding", buf);

    if( buf[0]!=0 )
    {
	UErrorCode errCode = U_ZERO_ERROR;
	dbConverter = ucnv_open( buf, &errCode );

	if( !U_SUCCESS( errCode ) )
	{
	    ERR( "Can't create converter from DB codepage(" << buf << "). ErrCode: " << (int)errCode );

	    throw DBQueryException( (int)errCode,
				    "Can't create converter from DB codepage",
				    std::string(buf) );
	}
    }

#ifdef OTL_ODBC
    buf[0]=0;

    CFGMAN::instance()->get_cstring_value("common", "db_isolation_level", buf);
    std::string dbIL = buf;

    if( dbIL == "read_uncommited" )
    {
	isolationLevel = otl_tran_read_uncommitted;
    }
    else if( dbIL == "read_commited" )
    {
	isolationLevel = otl_tran_read_committed;
    }
    else if( dbIL == "repeatable_read" )
    {
	isolationLevel = otl_tran_repeatable_read;
    }
    else if( dbIL == "serializable" )
    {
	isolationLevel = otl_tran_serializable;
    }
    else
    {
	isolationLevel = -1;
    }
#endif

    initialized = true;
}

void otl_wrapper_impl::otl_terminate( void )
{
    if ( !initialized )
	return;

    if( dbConverter )
	ucnv_close( dbConverter );

#ifndef OTL_ODBC
    otl_connect::otl_terminate();
#endif

    initialized = false;
}

#ifdef OTL_ODBC
# define WMC_DB_TYPE "ODBC"
#else
# define WMC_DB_TYPE "OCI"
#endif

extern "C" otl_connect_wrapper* new_otl_connect_wrapper
    ( const std::string& connectString ) throw( DBQueryException )
{
    DBG( 1, "new OTL connection for dbtype:" << WMC_DB_TYPE << " to '" << connectString << "'" );
    return new otl_connect_wrapper_impl( connectString );
}

extern "C" otl_wrapper* new_otl_wrapper( void )
{
    return new otl_wrapper_impl();
}
