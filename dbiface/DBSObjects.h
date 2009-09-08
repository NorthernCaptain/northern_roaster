#ifndef DBSOBJECTS_H_
#define DBSOBJECTS_H_

#include <map>
#include <list>
#include <string>

#include <boost/smart_ptr.hpp>

#include <ace/Synch.h>
#include <ace/Singleton.h>
#include <ace/Task.h>

//#define OTL_UTF8_MAX_BYTES 2

#include "otl_connect_wrapper.h"
#include "dbglog.h"
#include "DBQueryException.h"


typedef boost::shared_ptr< otl_connect_wrapper > otl_connect_ptr;

struct IsFreeConnect : std::unary_function< otl_connect_ptr, bool >
{
    bool operator()( const otl_connect_ptr& obj ) const;
};


class Session;//Из Session.h
/**
   Дом для соединений с одной базой из под одного пользователя БД. 
*/
class DBConnectionHome
{
private:
    std::list< otl_connect_ptr >             dbConnectionsList;
    std::string                              connectString;
    ACE_Thread_Mutex                         mutex;

public:
    DBConnectionHome();
    DBConnectionHome( const std::string& );
    ~DBConnectionHome( void );
    otl_connect_ptr getDBConnection( void ) throw ( DBQueryException );

    void badConnection( const otl_connect_ptr& );
    void getDBConnectionsInfo( size_t& freeConnectionCount, 
			       size_t& totalConnectionCount, 
			       std::string& connectStr );
    void releaseFreeConnections( short unsigned int& released );

};

extern "C" otl_connect_wrapper_func new_otl_connect_wrapper_f;

typedef ACE_Singleton<DBConnectionHome, ACE_Thread_Mutex>     DBIMAN;

#endif
