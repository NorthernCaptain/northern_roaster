#include <string.h>

#include "StringUtils.h"
#include "dbglog.h"

namespace StringUtils
{
    std::vector< std::string> makeListFromString( const std::string& SrcString, 
						const std::string& Separator,
						bool separatorIsAWord )
    {
//	DBG(0,"All: '"<<SrcString<<"\', length: "<<SrcString.length());
	std::vector< std::string > resultList;

	if( SrcString.empty() )
	    return resultList;




	size_t pos = separatorIsAWord ? SrcString.find( Separator ) : SrcString.find_first_of( Separator ), prevPos = 0;
	std::string tmp = "";

	if( pos == SrcString.npos )
	    resultList.push_back( SrcString );
	else
	{
	    while( pos != SrcString.npos )
	    {
		tmp = SrcString.substr( prevPos, pos - prevPos );

//	    	DBG(0,"tmp: '"<<tmp<<"\', pos: "<< pos<< ", prevPos: "<<prevPos);

		if( !tmp.empty() )
		    resultList.push_back( tmp );

		//tmp = SrcString.substr( prevPos, pos - prevPos );
		if( (pos+1) >= SrcString.length() )
		    break;

		prevPos = pos + 1;
		pos = separatorIsAWord ? SrcString.find(Separator,prevPos) : SrcString.find_first_of(Separator,prevPos);

		pos = (pos == SrcString.npos) ? SrcString.length() : pos;
	    }
	}

	return resultList;
    }

}

