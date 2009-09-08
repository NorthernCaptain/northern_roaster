#include <sstream>

#include <unicode/ucnv.h>
#include <unicode/ustring.h>

#include "dbglog.h"

//#define OTL_TRACE_LEVEL 0xff
//#define OTL_TRACE_STREAM *(DL.Out())

#include <otlv4.h>


//#include "Datetime.h"
#include "DBQueryException.h"
#include "CommonEnums.h"
#include "DBSObjects.h"
#include "Datetime.h"

#include "otl_connect_wrapper_impl.h"
#include "otl_stream_wrapper_impl.h"

otl_stream_wrapper_impl::otl_stream_wrapper_impl
    ( const otl_connect_ptr& dbConnect,
      int bufSize, 
      const std::string& query ) throw( DBQueryException )
    : rawStream( 0 ),
      lockedConnect( dbConnect )
{
    DBG( 15, "Connection '"<< lockedConnect.get() << "' now in use. Use count: " << lockedConnect.use_count() );

    if( lockedConnect.get() == 0 || lockedConnect->rawConnect() == 0 )
    {
	throw DBQueryException( 0, "Connect to DB pointer is null!", "" );
    }


    dbConverter = lockedConnect->getConverter();

    try
    {
	if ( query.empty() )
	    rawStream = new otl_stream();
	else
	    rawStream = new otl_stream( bufSize, query.c_str(), *(lockedConnect->rawConnect()) );
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error

	ERR( mess.str() );
	throw DBQueryException( otlE.code,
				"otl_stream_wrapper_imp() otl_exception caught:",
				mess.str() );
    }
}


otl_stream_wrapper_impl::~otl_stream_wrapper_impl()
{
    DBG( 15, "Connection '"<< lockedConnect.get() << "' will be released. Use count: " << lockedConnect.use_count() );

    try
    {
	if ( rawStream != 0 )
	    delete rawStream;
    }
    catch( const otl_exception& otlE )
    {//При закрытии может, например делаться commit и если в это время возникнет otl_exception его надо отловить
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_stream_wrapper_impl::~() otl_exception caught:",
				mess.str() );
    }
}

void otl_stream_wrapper_impl::open( int bufSize, 
				    const std::string& query ) throw( DBQueryException )
{
    if ( lockedConnect.get() == 0 || lockedConnect->rawConnect() == 0 )
    {
	ERR( "NULL raw otl connect pointer!" );
	return;
    }

    DBG( 15, "Connection '"<< lockedConnect.get() << "' will be released. Use count: " << lockedConnect.use_count() );

    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return;
    }

    try
    {
	rawStream->open( bufSize, query.c_str(), *(lockedConnect->rawConnect() ) );

	dbConverter = lockedConnect->getConverter();
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_stream_wrapper_impl::open() otl_exception caught:",
				mess.str() );
    }

}

int otl_stream_wrapper_impl::good()
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return 0;
    }

    return rawStream->good();
}

void otl_stream_wrapper_impl::flush() throw( DBQueryException )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return;
    }

    try
    {
	rawStream->flush();
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	throw DBQueryException( otlE.code,
				"otl_stream_wrapper_impl::flush() otl_exception caught:",
				mess.str() );
    }
}

int otl_stream_wrapper_impl::eof()
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return 0;
    }

    return rawStream->eof();
}

void otl_stream_wrapper_impl::close() throw( DBQueryException )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return;
    }

    try
    {
	rawStream->close();
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "otl_stream_wrapper_impl::close() otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }
}

void otl_stream_wrapper_impl::set_commit( int auto_commit )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return;
    }

    rawStream->set_commit( auto_commit );
}

void otl_stream_wrapper_impl::set_flush( const bool flush_flag )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return;
    }

    rawStream->set_flush( flush_flag );
}

int otl_stream_wrapper_impl::is_null()
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return 1;
    }

    return rawStream->is_null();
}

DataType otl_stream_wrapper_impl::get_next_out_var_type() throw( DBQueryException )
{
    if( !rawStream )
    {
        ERR( "NULL raw otl stream pointer!" );
        return NULL_VAL;
    }

    otl_var_desc* nvd = 0;

    try
    {
	nvd = rawStream->describe_next_out_var();
    }
    catch( const otl_exception& otlE )
    {
        std::ostringstream mess;

        mess << otlE.msg << std::endl; // print out error message
        mess << otlE.stm_text << std::endl; // print out SQL that caused the error
        mess << otlE.var_info; // print out the variable that caused the error

        DBQueryException dbqe( otlE.code,
                               "get_next_out_var_type(): otl_exception caught:",
                               mess.str() );
        throw dbqe;
    }

    if( nvd == 0 )
    {
        ERR( "Can't describe next variable from otl stream!" );
	return NULL_VAL;
    }

    switch( nvd->ftype )  
    {
    case otl_var_char:
    case otl_var_varchar_long:
    case otl_var_raw_long:
    case otl_var_clob:
    case otl_var_blob:
	return STRING_VAL;
    case otl_var_double:
    case otl_var_float:
	return DOUBLE_VAL;
    case otl_var_int:
    case otl_var_unsigned_int:
    case otl_var_short:
    case otl_var_long_int:
	return NUMBER_VAL;
    case otl_var_timestamp:
	return DATETIME_VAL;
    default:
	return NULL_VAL;
    }
}

otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( int toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) << toStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(int): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( long toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) << toStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(long): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( float toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) << toStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(float): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( double toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }

    try
    {    
	(*rawStream) << toStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(double): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
/*
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( const char* toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) << toStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "DBQuery:substValues() otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
*/
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( const std::string& toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	if( !dbConverter )//база отдает и принимает в utf-8
	{
	    (*rawStream) << toStream;
	    return *this;
	}

	UErrorCode errCode = U_ZERO_ERROR;
	
	int uSymbNum = toStream.size();
	
	UChar fromUN[ uSymbNum + 1 ];
	
	int uSymb = 0;
	
	u_strFromUTF8( fromUN, 
		       uSymbNum + 1, 
		       &uSymb, 
		       toStream.c_str(), 
		       -1,
		       &errCode );

	if( errCode != U_ZERO_ERROR )
	    ERR( "u_strFromUTF8 return ERROR, errCode = " << 
		 errCode << ", source string '" << toStream << '\'' );

	char destCP[ uSymb + 1 ];
	
	int len = ucnv_fromUChars( dbConverter, destCP, uSymb + 1, fromUN, -1, &errCode );

	if( errCode != U_ZERO_ERROR )
	    ERR( "ucnv_fromUChars return ERROR, errCode = " << errCode << ", position = " << len << 
		 ", source string '" << toStream << '\'' );
	
	std::string tmp( destCP, uSymb );
	
//	    DBG( 0, " val '" << tmp << '\'' );
	
	(*rawStream) << tmp;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(string): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( const otl_null_wrapper& )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) << otl_null();
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(null): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator<<( const Datetime& toStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	otl_datetime odt;
	toStream.initOTLDatetime( odt );
	(*rawStream) << odt;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator<<(Datetime): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}

otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( int& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) >> fromStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(int): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}

otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( long& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) >> fromStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(long): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}

otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( float& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) >> fromStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(float): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( double& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) >> fromStream;
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(double): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( std::string& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	(*rawStream) >> fromStream;

	if( !dbConverter )//база отдает и принимает в utf-8
	    return *this;

	UErrorCode errCode = U_ZERO_ERROR;
	int symbNum = fromStream.size();
	
	UChar destUN[ symbNum + 1 ];
	
	int len = ucnv_toUChars( dbConverter, destUN, symbNum + 1, fromStream.c_str(), -1, &errCode );
	
	if( errCode != U_ZERO_ERROR )
	    ERR( "ucnv_toUChars return ERROR, errCode = " << errCode << ", position = " << len << 
		 ", source string '" << fromStream << '\'' );

	char destUTF8[ symbNum * 6 + 1 ];
	int  utf8Bytes = 0;
	
	u_strToUTF8( destUTF8, 
		     symbNum * 6 + 1, 
		     &utf8Bytes, 
		     destUN, 
		     symbNum,
		     &errCode );

	if( errCode != U_ZERO_ERROR )
	    ERR( "u_strToUTF8 return ERROR, errCode = " << 
		 errCode << ", source string '" << fromStream << '\'' );
	
	fromStream = std::string( destUTF8, utf8Bytes );//, (colLen < unicodeSymbols ? colLen : unicodeSymbols) );

//	    DBG( 0, " val '" << fromStream << '\'' );
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(string): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}
otl_stream_wrapper_impl& otl_stream_wrapper_impl::operator>>( Datetime& fromStream )
{
    if( !rawStream )
    {
	ERR( "NULL raw otl stream pointer!" );
	return *this;
    }
    
    try
    {
	otl_datetime tmp;
	(*rawStream) >> tmp;
	fromStream = Datetime( tmp );
    }
    catch( const otl_exception& otlE )
    {
	std::ostringstream mess;
	
	mess << otlE.msg << std::endl; // print out error message
	mess << otlE.stm_text << std::endl; // print out SQL that caused the error
	mess << otlE.var_info; // print out the variable that caused the error
	
	DBQueryException dbqe( otlE.code,
			       "operator>>(Datetime): otl_exception caught:",
			       mess.str() );
	throw dbqe;
    }

    return *this;
}

