// -*- C++ -*-

#ifndef __DATETIME_H_
#define __DATETIME_H_

#include <string>

class DatetimeConvertException
{
public:
    DatetimeConvertException( const std::string& message ) : Message( message ) {}

    std::string Message;
};

class otl_datetime;

/** Структура для передачи даты от базы к корбе*/
class Datetime {
public:
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;

    Datetime( void ) {}

    Datetime( const otl_datetime& o );

    Datetime( const struct tm* stm )
	: year( stm->tm_year + 1900 ),
	  month( stm->tm_mon + 1 ),
	  day( stm->tm_mday ),
	  hour( stm->tm_hour ),
	  minute( stm->tm_min ),
	  second( stm->tm_sec )
    {}

    Datetime(time_t tim);

    bool operator==( const Datetime& rh ) const
    {
	return year   == rh.year && 
	       month  == rh.month && 
	       day    == rh.day && 
	       hour   == rh.hour &&
	       minute == rh.minute &&
	       second == rh.second;
    }

    void initOTLDatetime( otl_datetime& otlDT ) const;

    int  toInt( void ) const throw( DatetimeConvertException );
};

#endif
