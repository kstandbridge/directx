#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		13/06/2018 - Lenningen - Luxembourg
*
* Desc:		the states / scenes of a game
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <unordered_map>

// bell0bytes utilities
#include "observer.h"

// event queue
#include "depesche.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace graphics
{
	class Direct2D;
}

namespace core
{
	class DirectXApp;

	class GameState : public core::DepescheDestination, public core::DepescheSender
	{
	private:
		const std::wstring name;				// the name of the scene
				
	protected:
		DirectXApp& dxApp;						// adress of the main application class (can't be "const" because the stack might change)
		const graphics::Direct2D& d2d;			// pointer to the Direct2D object of the DirectXApp


		bool firstCreation;						// boolean to make sure the fixed layouts are not created more than once
		
		// protected constructor -> singleton
		GameState(DirectXApp& app, const std::wstring& name);

	public:
		virtual ~GameState();

		bool isPaused;							// true iff the scene is paused

		// delete copy and assignment operators
		GameState(GameState const &) = delete;
		GameState& operator = (GameState const &) = delete;
		
		// initialization
		virtual util::Expected<void> initialize() = 0;
		virtual util::Expected<void> shutdown() = 0;

		// pause and resume
		virtual util::Expected<void> pause() = 0;
		virtual util::Expected<void> resume() = 0;

		// user input
		virtual util::Expected<void> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) = 0;	// returns false if the observer stack of the input handler was changed
		virtual util::Expected<void> update(const double deltaTime) = 0;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) = 0;

		// handle events
		virtual util::Expected<void> onMessage(const Depesche&) = 0;
	};
}
