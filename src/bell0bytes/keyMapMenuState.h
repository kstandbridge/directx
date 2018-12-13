#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		17/06/2018 - Lenningen - Luxembourg
*
* Desc:		menu to change key map
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "states.h"

#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wrl.h>

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace graphics
{
	class AnimatedSprite;
}

namespace audio
{
	struct SoundEvent;
}

namespace UI
{
	class AnimatedButton;

	class KeyMapMenuState : public core::GameState
	{
	private:
		// a new brush to draw the background of the overlay rectangle
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush;
		
		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> titleFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> headerFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> textFormat;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> titleLayout;
		std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout4> > actionTextLayouts;
		std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout4> > keyBindings1TextLayouts;
		std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout4> > keyBindings2TextLayouts;
		std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout4> > headerTextLayouts;

		// text strings
		std::vector<std::wstring> keyBindings1Texts;
		std::vector<std::wstring> keyBindings2Texts;

		// the menu buttons
		std::deque<AnimatedButton*> menuButtons;
		int currentlySelectedButton;
		audio::SoundEvent* buttonClickSound;
		
		// list of actions (paged)
		unsigned int currentPage;
		const unsigned int keyBindingsPerPage;

		// initialize texts and layouts
		util::Expected<void> createTextFormats();
		util::Expected<void> createHeaderLayouts();
		util::Expected<void> recreateLayouts();
		void releaseAndClearLayouts();

		// populate vectors
		util::Expected<void> addKeyBindingToLayoutList(const unsigned int i, input::GameCommands);
		util::Expected<void> addTextToActionTextLayoutList(input::GameCommands);
		util::Expected<void> createGamepadButton(unsigned int i);

		// initialize buttons
		util::Expected<void> initializeButtons();

	protected:
		KeyMapMenuState(core::DirectXApp& app, const std::wstring& name);

	public:
		virtual ~KeyMapMenuState();

		// singleton: get instance
		static KeyMapMenuState& createInstance(core::DirectXApp& app, const std::wstring& name);

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

		// change key binding
		util::Expected<void> changeKeyBinding();

		// handle message
		virtual util::Expected<void> onMessage(const core::Depesche&) override;
	};
}