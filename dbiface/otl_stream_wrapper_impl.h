#ifndef OTL_STREAM_WRAPPER_IMPL_H_
#define OTL_STREAM_WRAPPER_IMPL_H_

#include "otl_stream_wrapper.h"

class otl_connect_wrapper_impl;

class otl_stream_wrapper_impl : public otl_stream_wrapper
{
private:
    otl_stream* rawStream;

    UConverter* dbConverter;

    otl_connect_ptr lockedConnect;

    otl_stream_wrapper_impl( const otl_stream_wrapper_impl& ); //nocopy
    otl_stream_wrapper_impl& operator=( const otl_stream_wrapper_impl& ); //nocopy

protected:
    friend class otl_connect_wrapper_impl;
    otl_stream_wrapper_impl( const otl_connect_ptr& dbConnect,
			     int bufSize = 0, 
			     const std::string& query = "" ) throw( DBQueryException );
public:
    virtual ~otl_stream_wrapper_impl();

    virtual void open( int bufSize, const std::string& query ) throw( DBQueryException );
    virtual int  good( void );
    virtual int  eof( void );
    virtual void flush( void ) throw( DBQueryException );
    virtual void close( void ) throw( DBQueryException );
    virtual void set_commit( int auto_commit );
    virtual void set_flush( const bool flush_flag = true );
    virtual int  is_null( void );

    virtual DataType get_next_out_var_type( void ) throw( DBQueryException );

    virtual otl_stream_wrapper_impl& operator<<( const int toStream );
    virtual otl_stream_wrapper_impl& operator<<( const long toStream );
    virtual otl_stream_wrapper_impl& operator<<( const float toStream );
    virtual otl_stream_wrapper_impl& operator<<( const double toStream );
//  virtual otl_stream_wrapper_impl& operator<<( const char* toStream );
    virtual otl_stream_wrapper_impl& operator<<( const std::string& toStream );
    virtual otl_stream_wrapper_impl& operator<<( const otl_null_wrapper& toStream );
    virtual otl_stream_wrapper_impl& operator<<( const Datetime& toStream );

    virtual otl_stream_wrapper_impl& operator>>( int& fromStream );
    virtual otl_stream_wrapper_impl& operator>>( long& fromStream );
    virtual otl_stream_wrapper_impl& operator>>( float& fromStream );
    virtual otl_stream_wrapper_impl& operator>>( double& fromStream );
    virtual otl_stream_wrapper_impl& operator>>( std::string& fromStream );
    virtual otl_stream_wrapper_impl& operator>>( Datetime& fromStream );

};

#endif
