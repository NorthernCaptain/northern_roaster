#ifndef	__DBQUERYEXCEPTION_H
#define	__DBQUERYEXCEPTION_H

#include <string>

class DBQueryException
{
public:
    std::string errMesID;
    int         ErrCode;
    std::string ServerMessage;
    std::string ToolMessage;

    DBQueryException( int errCode, 
		      const std::string& serverMessage, 
		      const std::string& toolMessage,
		      const std::string& errMes = "m.dbquery.exception" );

    std::string getLocalizedMessage( const std::string& localeID ) const;
};

#endif
