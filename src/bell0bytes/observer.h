#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		28/05/2018 - Lenningen - Luxembourg
*
* Desc:		The Observer pattern
*
* History:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <unordered_set>

// bell0bytes util
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace input
{
	enum GameCommands : int;
	struct GameCommand;
	struct BindInfo;
	class InputHandler;
}

namespace util
{
	class Observer
	{
	public:
		virtual ~Observer() {};
		
		virtual Expected<void> onNotify(const int) { return {}; };
	};

	class Subject
	{
	private:
		std::unordered_set<Observer*> observers;					// a set of observers				

	protected:
		Expected<void> notify(const int) const;

	public:
		Subject() {};
		~Subject() {};

		// add or remove observers
		void addObserver(Observer* const observer);
		void removeObserver(Observer* const observer);

		// get number of observers (for debugging)
		auto getNumberOfObservers() { return observers.size(); };
	};
}
