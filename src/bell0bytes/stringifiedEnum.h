#pragma once

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/wstringize.hpp>

namespace util
{
	// use boost to define enums with strings

	// this first macro is used internally by the second one
#define ENUM_TO_STRING(r, data, elem)											\
    case elem : return BOOST_PP_WSTRINGIZE(elem);

	// this second macro first generates the enumeration and then generates a ToString function
	// that takes an object of that type and returns the enumerator name as a string
	// obviously this implementation requires that the enumerators map to unique values)
#define ENUM_WITH_STRING(name, enumerators)										\
    enum name {																	\
        BOOST_PP_SEQ_ENUM(enumerators)											\
    };																			\
																				\
    inline const wchar_t* enumToString(name v)									\
    {																			\
        switch (v)																\
        {																		\
            BOOST_PP_SEQ_FOR_EACH(												\
                ENUM_TO_STRING,													\
                name,															\
                enumerators														\
            )																	\
            default: return "[Unknown " BOOST_PP_WSTRINGIZE(name) "]";			\
        }																		\
    }
}