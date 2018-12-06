#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		03/09/2017 - Lenningen - Luxembourg
*
* Desc:		graphics related helper functions
*
* History:
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and Com
#include <wrl/client.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

namespace graphics
{
	// function to create a "random" float between 0.0f and 1.0f
	static inline float randomColour()
	{
		return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
	}

	// function to create a "random" float between -1.0f and 1.0f
	static inline float randomPosition()
	{
		return  static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)) * 2 - 1;
	}
}