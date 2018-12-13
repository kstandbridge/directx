#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		17/06/2018 - Lenningen - Luxembourg
*
* Desc:		change key binding
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes core
#include "states.h"

#include <wrl.h>
#include <dwrite_3.h>
#include <d2d1_3.h>

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

	class NewKeyBindingState : public core::GameState
	{
	private:
		// background brush
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> whiteBrush;

		// text formats
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> titleFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> eventFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> oldKeyBindingFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> newKeyBindingFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> pressKeyFormat;
		Microsoft::WRL::ComPtr<IDWriteTextFormat3> pressEscapeKeyFormat;

		// text layouts
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> titleLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> eventLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> oldKeyBindingLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> newKeyBindingLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> pressKeyLayout;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> pressEscapeKeyLayout;
					
		// the menu buttons
		std::deque<AnimatedButton*> menuButtons;
		int currentlySelectedButton;
		audio::SoundEvent* buttonClickSound;

		// current key binding and game action
		input::GameCommands gameCommand;				// the game action, i.e. "back", "select", ...
		input::GameCommand* commandToChange;			// the actual game command object to change
		std::wstring oldKeyBinding;						// the string of the old chord of the game command
		std::vector<input::BindInfo> newChord;			// the new chord for the game command

		// listen for user input
		bool keySelected;				// stop listening once the user did create a new chord
		bool showPressKey;				// show the "press key" message if and only if the user has not yet selected a new chord

		// function to set the new chord for the game command stored above
		util::Expected<void> setNewChord(std::vector<input::BindInfo>&);

		// helper functions
		util::Expected<void> createTextFormats();
		util::Expected<void> createTextLayouts();

	protected:
		// overload constructor to get the game command object to change
		NewKeyBindingState(core::DirectXApp& app, std::wstring name);
		NewKeyBindingState(core::DirectXApp& app, std::wstring name, input::GameCommands, std::wstring oldKeyBinding, input::GameCommand* = NULL);

	public:
		virtual ~NewKeyBindingState();

		// singleton: get instance
		static NewKeyBindingState& createInstance(core::DirectXApp& app, std::wstring name);
		
		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		util::Expected<void> initializeButtons();				// initialize the menu button graphics

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<void> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;

		// setters to allow the KeyMapMenuState class to set/specify the actual game command object to be changed
		void setOldKeyBindingString(std::wstring oldKeyBindingString) { this->oldKeyBinding = oldKeyBindingString; };
		void setGameCommand(input::GameCommands gc) { this->gameCommand = gc; };
		void setCommandToChange(input::GameCommand* command) { this->commandToChange = command; };

		// handle message
		virtual util::Expected<void> onMessage(const core::Depesche&) override;
	};
}