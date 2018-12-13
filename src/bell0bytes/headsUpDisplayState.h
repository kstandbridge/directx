#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		21/06/2018 - Lenningen - Luxembourg
*
* Desc:		heads up display
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

namespace graphics
{
	class AnimatedSprite;
}

namespace UI
{
	class HeadsUpDisplayState : public core::GameState
	{
	private:
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> hudFormat;			// dummy text
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> hudLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> batteryLevelFormat;	// gamepad battery level
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> batteryLevelLayout;

		// keep track of the number of cats
		unsigned int nActiveCats = 5;

		// battery level icon
		graphics::AnimatedSprite* iconBatteryLevel;

		HeadsUpDisplayState(core::DirectXApp& dxApp, const std::wstring& name);
		util::Expected<void> onMessage(const core::Depesche&) override;

	public:
		virtual ~HeadsUpDisplayState();

		// singleton: get instance
		static HeadsUpDisplayState& createInstance(core::DirectXApp& dxApp, const std::wstring& name);

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
	};
}