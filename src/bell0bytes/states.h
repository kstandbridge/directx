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

// bell0bytes utilities
#include "observer.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace graphics
{
	class Direct2D;
}

namespace core
{
	class DirectXApp;

	class GameState : public util::Observer
	{
	private:
		std::wstring name;						// the name of the scene
				
	protected:
		DirectXApp* const dxApp;				// pointer to the main application class
		graphics::Direct2D* const d2d;			// pointer to the Direct2D object of the DirectXApp

		bool isPaused;							// true iff the scene is paused

		// protected constructor -> singleton
		GameState(DirectXApp* const app, std::wstring& name);

	public:
		virtual ~GameState();

		// delete copy and assignment operators
		GameState(GameState const &) = delete;
		GameState& operator = (GameState const &) = delete;
		
		// initialization
		virtual void initialize() = 0;
		virtual void shutdown() = 0;

		// pause and resume
		virtual void pause() = 0;
		virtual void resume() = 0;

		// user input
		virtual bool handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) = 0;	// returns false if the observer stack of the input handler was changed
		virtual void update(const double deltaTime) = 0;

		// render the scene
		virtual void render(const double farSeer) = 0;

		// change to another scene
		void changeState(GameState* const gameState);

		// get name
		std::wstring& getStateName() { return name; };
	};
}
