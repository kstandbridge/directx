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

// bell0bytes includes
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace util
{
	class Observer
	{
	public:
		virtual ~Observer() {};
		
		virtual util::Expected<void> onNotify(const int) = 0;
	};

	class Subject
	{
	private:
		std::set<Observer*> observers;					// a set of observers				

	protected:
		util::Expected<void> notify(const int) const;

	public:
		Subject() {};
		~Subject() {};

		// add or remove observers
		void addObserver(Observer* const observer);
		void removeObserver(Observer* const observer);
	};
}
