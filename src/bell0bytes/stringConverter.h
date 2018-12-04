#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		08/08/2017 - Eichlinghofen - Germany
*
* Desc:		converts string to wstring and vice-versa
*
* History:
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

#include <string>							// strings
#include <locale>							// immutable indexed set of immutable facets; each stream object of the C++ input/output library is associated with a std::locale object and uses its facets for parsing and formatting of all data
#include <codecvt>							// encapsulates conversion of character strings, including wide and multibyte, from one encoding to another 

namespace util
{

// CLASSES //////////////////////////////////////////////////////////////////////////////

	class StringConverter
	{
	public:
		static std::wstring s2ws(const std::string& str);
		static std::string ws2s(const std::wstring& ws);
	};
}