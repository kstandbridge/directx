#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		game menu (pause game, for example)
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes includes
#include "states.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace UI
{
	class GameMenuState : public core::GameState
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> gameMenuFormat;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> gameMenuLayout;

	protected:
		GameMenuState(core::DirectXApp* const app, std::wstring name);

	public:
		virtual ~GameMenuState();

		// singleton: get instance
		static GameMenuState& createInstance(core::DirectXApp* const app, std::wstring name);

		// observer: on notification
		util::Expected<bool> onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<bool> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;
	};
}