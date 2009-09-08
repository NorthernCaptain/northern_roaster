#include <ace/OS.h>

#include <otlv4.h>

#include "Datetime.h"

//#include "dbglog.h"

Datetime::Datetime( const otl_datetime& o )
    : year( o.year ),
      month( o.month ),
      day( o.day ),
      hour( o.hour ),
      minute( o.minute ),
      second( o.second )
{}

Datetime::Datetime(time_t tim)
{
    struct tm stm;
    localtime_r(&tim, &stm);
    year = stm.tm_year + 1900;
    month = stm.tm_mon + 1;
    day = stm.tm_mday;
    hour = stm.tm_hour;
    minute = stm.tm_min;
    second = stm.tm_sec;
}

void Datetime::initOTLDatetime( otl_datetime& otlDT ) const
{
      otlDT.year   = year;
      otlDT.month  = month;
      otlDT.day    = day;
      otlDT.hour   = hour;
      otlDT.minute = minute;
      otlDT.second = second;
}

int Datetime::toInt( void ) const throw( DatetimeConvertException )
{
    struct tm stm;
    
    stm.tm_year = year - 1900;// >= 1900 ? year - 1900 : year;
    stm.tm_mon  = month - 1;// >= 1 ? month - 1 : month;
    stm.tm_mday = day;
    stm.tm_hour = hour;
    stm.tm_min  = minute;
    stm.tm_sec  = second;
    stm.tm_isdst = -1;//Предоставить учет перехода на зимнее/летнее время системе
//    DBG( 0, "Y:" << stm.tm_year << " M:" << stm.tm_mon << " D:" << stm.tm_mday << " H:" << stm.tm_hour 
//	 << " M:" << stm.tm_min << " S:" << stm.tm_sec );


    time_t result;
    
    result = ACE_OS::mktime( &stm );
    
    if( result == (time_t)(-1) )
	throw DatetimeConvertException( "Datetime::toInt(): Can't convert." );
    
//    DBG( 0, (int)result );

    return (int)result;
}

