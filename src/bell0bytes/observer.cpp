#include "observer.h"

namespace util
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// The Observer /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// The Subject //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Subject::addObserver(Observer* const observer)
	{
		// add observer to observer list
		observers.insert(observer);
	}

	void Subject::removeObserver(Observer* const observer)
	{
		// remove the observer from the list
		observers.erase(observer);
	}

	util::Expected<void> Subject::notify(const int event) const
	{
		util::Expected<void> notificationResult;

		for (auto x : observers)
		{
			notificationResult = x->onNotify(event);
			if (!notificationResult.isValid())
				return notificationResult;
		}

		return { };
	}

	util::Expected<void> Subject::notify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) const
	{
		util::Expected<bool> notificationResult = true;
		
		for(auto x : observers)
		{
			notificationResult = x->onNotify(activeKeyMap);
			if (!notificationResult.isValid())
				return notificationResult;
			else
				if (!notificationResult.get())
					// the game state has changed -> break
					break;
		}

		// return success
		return { };
	}
}