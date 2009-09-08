#include <string>
#include <vector>

#ifndef	__STRING_UTILS_H
#define	__STRING_UTILS_H

namespace StringUtils
{
    /**
     * ��������� ���� ����� �� ������, �������� �� �� �����������.
     * @param separatorIsAWord ������� ����, ��� � �������� ����������� ���� 
     * ������������� ���� Separator �������, � �� �����������
     */
    std::vector<std::string> makeListFromString( const std::string& SrcString, 
						 const std::string& Separator,
						 bool separatorIsAWord = true );
}


#endif
