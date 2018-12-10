#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		main state of the running game
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <iostream>

// bell0bytes includes
#include "states.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;

	class PlayState : public GameState
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> playStateFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> playStateLayout;
				
	protected:
		PlayState(DirectXApp* const app, std::wstring name);

	public:
		virtual ~PlayState();

		// singleton: get instance
		static PlayState& createInstance(DirectXApp* const app, std::wstring name);

		// observer: on notification
		util::Expected<bool> onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;

		// initialization
		virtual void initialize() override;
		virtual void shutdown() override;

		// pause and resume
		virtual void pause() override;
		virtual void resume() override;

		// user input
		virtual bool handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual void update(const double deltaTime) override;

		// render the scene
		virtual void render(const double farSeer) override;
	};
}