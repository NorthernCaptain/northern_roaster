#include "DBQueryException.h"
#include "dbglog.h"

DBQueryException::DBQueryException( int errCode, 
				    const std::string& serverMessage, 
				    const std::string& toolMessage,
				    const std::string& errMes)
    : errMesID( errMes ),
      ErrCode( errCode ),
      ServerMessage( serverMessage ),
      ToolMessage( toolMessage )
{

    DBG(4, "ERROR: " << getLocalizedMessage(""));
}



std::string DBQueryException::getLocalizedMessage( const std::string& localeID ) const
{
    return std::string("DataBase Query Exception:")
	+ errMesID + ":" + ToolMessage + " ("
	+ ServerMessage + ")";
}
