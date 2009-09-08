#include <iostream>
#include <sstream>

#include "DBSObjects.h"
//#include "Session.h"

#include "dbglog.h"
#include "configman.hxx"

// extern "C"
otl_connect_wrapper_func new_otl_connect_wrapper_f = 0;

bool IsFreeConnect::operator()( const otl_connect_ptr& obj ) const
{
//    DBG( 15, "Use count: " << obj.use_count() << " Ptr: " << obj.get() );
    return ( (obj.use_count() == 1) && (obj.get() != 0) );
}

/**
   Конструктор для дома DBConnection'ов.
   Просто инициализирует {@link #connectString}
   
   @param connectStr параметры соединения
*/
DBConnectionHome::DBConnectionHome( const std::string& connectStr )
{
    connectString = connectStr;
}

DBConnectionHome::DBConnectionHome()
{
    ACE_TCHAR  buf[128]="leo/????@unknown";
    CFGMAN::instance()->get_cstring_value("common", "db_connect", buf);
    CFGMAN::instance()->set_cstring_value("common", "db_connect", buf);
    connectString = buf;
}

/**
   Деструктор для дома DBConnection'ов.
*/
DBConnectionHome::~DBConnectionHome()
{
    // Может все таки залочить mutex?

//     std::list< otl_connect* >::iterator i = freeDBConnections.begin();
//     size_t count = 0;
    
//     while( i != freeDBConnections.end() )
//     {
// 	utilizeConnection( *i );
// 	i++;
// 	count++;
//     }

    DBG( 10, "DBConnectionHome deleted. " << dbConnectionsList.size() << " connections utilized." );
}

/**
   Возвращает указатель на готовый объект otl_connect. 
   Если готового otl_connect'а нет, то создает его.
*/
otl_connect_ptr DBConnectionHome::getDBConnection() throw ( DBQueryException )
{
    otl_connect_ptr res;
    ACE_Guard<ACE_Thread_Mutex> guard( mutex );

    std::list< otl_connect_ptr >::iterator resIt = find_if( dbConnectionsList.begin(),
							    dbConnectionsList.end(),
							    IsFreeConnect() );//Первый свободный 

    if( resIt != dbConnectionsList.end() )
    {// Есть уже готовый DBConnection
	res = *resIt;

	DBG( 15, "Return connection '"<< res.get() << "' (from pool). Use count: " << res.use_count() );
    }
    else
    {
	if ( new_otl_connect_wrapper_f == 0 )
	{
	    ERR( "DB library not loaded!" );
	    throw DBQueryException( 0, "DB library not loaded!", connectString );
	}

	res = otl_connect_ptr( new_otl_connect_wrapper_f( connectString ) );
//	res.reset( new_otl_connect_wrapper_f( connectString ) );

	dbConnectionsList.push_back( res );

	DBG( 15, "Return connection '"<< res.get() << "' (new). Use count: " << res.use_count() );
    }

    return res;
}

/**
   Удаляет объект otl_connect из листа.
*/
void DBConnectionHome::badConnection( const otl_connect_ptr& toUtil )
{
    DBG( 9, "Connection may be corrupted: " << toUtil.get() );

    if( toUtil == 0 )
	return;

    mutex.acquire();

    dbConnectionsList.remove( toUtil );

    mutex.release();

    DBG( 9, "Connection " << toUtil.get() << " removed from list." );
}


void DBConnectionHome::releaseFreeConnections( short unsigned int& released )
{
    mutex.acquire();

    size_t oldSize = dbConnectionsList.size();
    dbConnectionsList.remove_if( IsFreeConnect() );
    size_t newSize = dbConnectionsList.size();

    mutex.release();

    released = ( newSize > oldSize ) ? 0 : oldSize - newSize;
}

/**
   Возвращает кол-во конекций в спуле и строку подсоединения.
*/
void DBConnectionHome::getDBConnectionsInfo( size_t& freeConnectionCount, 
					     size_t& totalConnectionCount, 
					     std::string& connectStr )
{
    mutex.acquire();
    freeConnectionCount = count_if( dbConnectionsList.begin(),
				    dbConnectionsList.end(),
				    IsFreeConnect() );
    totalConnectionCount = dbConnectionsList.size();
    mutex.release();

    connectStr = connectString;
}


// /**
//    Помещает указатель на объект otl_connect обратно домой.
// */
// void DBConnectionHome::goHome( otl_connect*& toHome )
// {
//     if( toHome == 0 )
// 	return;

//     mutex.acquire();
//     freeDBConnections.push_back( toHome );
//     mutex.release();

//     DBG( 15, "Connection go home." );

//     toHome = 0;//не надо им больше пользоваться.
// }
