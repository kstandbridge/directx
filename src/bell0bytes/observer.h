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
#include <set>
#include <unordered_map>

// bell0bytes includes
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
		
		virtual util::Expected<void> onNotify(const int) { return {}; };
		virtual util::Expected<bool> onNotify(input::InputHandler* const /*bi*/, const bool /*listening*/) { return true; };
	};

	class Subject
	{
	private:
		std::set<Observer*> observers;					// a set of observers				

	protected:
		util::Expected<void> notify(const int) const;
		util::Expected<void> notify(input::InputHandler* const ih, const bool) const;

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
