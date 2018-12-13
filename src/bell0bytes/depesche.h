#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		28/06/2018 - Lenningen - Luxembourg
*
* Desc:		structure to define game events on the event queue
*			"Depesche" is the German word for telegram
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes util
#include "expected.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	enum DepescheTypes { ActiveKeyMap, Gamepad, Damage, PlaySoundEvent, StopSoundEvent, BeginStream, EndStream };

	class DepescheSender;
	class DepescheDestination;

	struct Depesche
	{
		DepescheSender* const sender;				// the sender of the message
		DepescheDestination* const destination;		// the destined receiver of the message
		const DepescheTypes type;					// the type of the message
		void* const message;						// the actual message

		Depesche();
		Depesche(DepescheSender&, DepescheDestination&, const DepescheTypes, void* const);
		~Depesche();
	};

	class DepescheSender
	{

	};

	class DepescheDestination
	{
	private:

	public:
		virtual util::Expected<void> onMessage(const Depesche&) = 0;// { return { }; }	// handle events
	};
}