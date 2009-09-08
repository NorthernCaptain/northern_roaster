#ifndef OTL_STREAM_WRAPPER_H_
#define OTL_STREAM_WRAPPER_H_

#include "CommonEnums.h"
#include "DBQueryException.h"

class Datetime;

class otl_null_wrapper
{
public:
    otl_null_wrapper() {}
    ~otl_null_wrapper() {}
};

class otl_stream_wrapper
{
public:
    virtual ~otl_stream_wrapper() {}

    virtual void open( int bufSize,
		       const std::string& query ) throw( DBQueryException ) = 0;
    virtual int  good( void ) = 0;
    virtual int  eof( void ) = 0;
    virtual void flush( void ) throw( DBQueryException ) = 0;
    virtual void close( void ) throw( DBQueryException ) = 0;
    virtual void set_commit( int auto_commit ) = 0;
    virtual void set_flush( const bool flush_flag = true ) = 0;
    virtual int  is_null( void ) = 0;

    virtual DataType get_next_out_var_type( void ) throw( DBQueryException ) = 0;

    virtual otl_stream_wrapper& operator<<( const int toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const long toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const float toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const double toStream ) = 0;
//  virtual otl_stream_wrapper& operator<<( const char* toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const std::string& toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const otl_null_wrapper& toStream ) = 0;
    virtual otl_stream_wrapper& operator<<( const Datetime& toStream ) = 0;

    virtual otl_stream_wrapper& operator>>( int& fromStream ) = 0;
    virtual otl_stream_wrapper& operator>>( long& fromStream ) = 0;
    virtual otl_stream_wrapper& operator>>( float& fromStream ) = 0;
    virtual otl_stream_wrapper& operator>>( double& fromStream ) = 0;
    virtual otl_stream_wrapper& operator>>( std::string& fromStream ) = 0;
    virtual otl_stream_wrapper& operator>>( Datetime& fromStream ) = 0;

};

#endif
