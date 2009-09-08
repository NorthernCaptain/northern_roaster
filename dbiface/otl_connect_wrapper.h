#ifndef OTL_CONNECT_WRAPPER_H_
#define OTL_CONNECT_WRAPPER_H_

#include "DBQueryException.h"

#include <boost/smart_ptr.hpp>

#include "otl_stream_wrapper.h"

/** Перечисление для описания типов курсора*/
enum CURSOR_TYPE
{
    WRAP_CURSOR_FORWARD_ONLY,
    WRAP_CURSOR_STATIC,
    WRAP_CURSOR_KEYSET_DRIVEN,
    WRAP_CURSOR_DYNAMIC,
    WRAP_CURSOR_DEFAULT
};

class UConverter;
class otl_connect;

/**
 * Абстрактный класс для соединения с БД
 */

class otl_connect_wrapper : public boost::enable_shared_from_this< otl_connect_wrapper >
{
public:
    virtual ~otl_connect_wrapper() = 0;

    virtual otl_connect* rawConnect() const = 0;

    virtual void commit( void ) throw ( DBQueryException ) = 0;
    virtual void rollback( void ) throw ( DBQueryException ) = 0;

    virtual void set_cursor_type( const CURSOR_TYPE acursor_type = WRAP_CURSOR_FORWARD_ONLY ) = 0;

    virtual otl_stream_wrapper* new_stream
	( int bufSize = 0, 
	  const std::string& query = "" ) throw ( DBQueryException ) = 0;

    virtual UConverter* getConverter( void ) const = 0;
};

typedef boost::shared_ptr<otl_stream_wrapper> otl_stream_wrapper_ptr;

/**
 * Абстрактный класс для инициализации БД
 */
class otl_wrapper
{
public:
    virtual ~otl_wrapper() {}
    virtual void otl_initialize( void ) = 0;
    virtual void otl_terminate( void ) = 0;
};

extern "C" otl_connect_wrapper* new_otl_connect_wrapper
    ( const std::string& connectString = "" ) throw( DBQueryException );

extern "C" otl_wrapper* new_otl_wrapper( void );

extern "C"
{
    typedef otl_connect_wrapper* (*otl_connect_wrapper_func) ( const std::string& );
    typedef otl_wrapper* (*otl_wrapper_func)( void );
}

#endif
