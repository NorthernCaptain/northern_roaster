#include <string>
#include <vector>

#ifndef	__STRING_UTILS_H
#define	__STRING_UTILS_H

namespace StringUtils
{
    /**
     * Формирует лист строк из строки, разбивая ее по разделителю.
     * @param separatorIsAWord признак того, что в качестве разделителя надо 
     * рассматривать весь Separator целиком, а не посимвольно
     */
    std::vector<std::string> makeListFromString( const std::string& SrcString, 
						 const std::string& Separator,
						 bool separatorIsAWord = true );
}


#endif
