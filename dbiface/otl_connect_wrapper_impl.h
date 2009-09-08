#ifndef OTL_CONNECT_WRAPPER_IMPL_H_
#define OTL_CONNECT_WRAPPER_IMPL_H_

#include "otl_stream_wrapper.h"
#include "otl_connect_wrapper.h"

class otl_connect_wrapper_impl : public otl_connect_wrapper
{
    otl_connect* rawConnect_;

    otl_connect_wrapper_impl( const otl_connect_wrapper_impl& ); // nocopy
    otl_connect_wrapper_impl& operator=( const otl_connect_wrapper_impl& ); // nocopy

public:
    otl_connect_wrapper_impl( const std::string& connectString = "" ) throw( DBQueryException );

    virtual otl_connect* rawConnect() const { return rawConnect_; }

    virtual ~otl_connect_wrapper_impl();

    virtual void commit( void ) throw( DBQueryException );
    virtual void rollback( void ) throw( DBQueryException );

    virtual void set_cursor_type( const CURSOR_TYPE acursor_type = WRAP_CURSOR_FORWARD_ONLY );

    virtual otl_stream_wrapper* new_stream
	( int bufSize = 0, 
	  const std::string& query = "" ) throw ( DBQueryException );

    virtual UConverter* getConverter( void ) const;
};

class otl_wrapper_impl : public otl_wrapper
{
public:
    virtual void otl_initialize( void );
    virtual void otl_terminate( void );
};

#endif
