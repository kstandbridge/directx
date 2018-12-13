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
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> gameMenuFormat;	// dummy text
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> gameMenuLayout;

		GameMenuState(core::DirectXApp& app, const std::wstring& name);

	public:
		virtual ~GameMenuState();

		// singleton: get instance
		static GameMenuState& createInstance(core::DirectXApp& app, const std::wstring& name);

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<void> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;

		// handle message
		virtual util::Expected<void> onMessage(const core::Depesche&) override;
	};
}