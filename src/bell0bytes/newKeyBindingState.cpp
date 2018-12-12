// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <sstream>

// bell0bytes core
#include "app.h"

// bell0bytes UI
#include "NewKeyBindingState.h"
#include "buttons.h"
#include "keyMapMenuState.h"
#include "newKeyBindingState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "sprites.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	NewKeyBindingState::NewKeyBindingState(core::DirectXApp* const app, std::wstring name) : GameState(app, name), currentlySelectedButton(0)
	{ }

	NewKeyBindingState::NewKeyBindingState(core::DirectXApp* const app, std::wstring name, input::GameCommands gameCommand, std::wstring oldKeyBinding, input::GameCommand* oldCommand) : GameState(app, name), currentlySelectedButton(0), gameCommand(gameCommand), oldKeyBinding(oldKeyBinding), commandToChange(oldCommand), keySelected(false), showPressKey(true)
	{ }

	NewKeyBindingState::~NewKeyBindingState()
	{ }

	NewKeyBindingState& NewKeyBindingState::createInstance(core::DirectXApp* const app, std::wstring stateName)
	{
		static NewKeyBindingState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// add to observer list of the input handler
		dxApp->addInputHandlerObserver(this);

		// tell the input system to listen for user input
		dxApp->enableListeningForInput();

		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp->getCurrentWidth() / 2, dxApp->getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// hide the standard cursor
		ShowCursor(false);

		// allow mouse input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = false;

		if (firstCreation)
		{
			// create brush
			d2d->createSolidColourBrush(D2D1::ColorF::WhiteSmoke, whiteBrush);
		}

		// create text formats
		if (!createTextFormats().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text formats for the new key bindings menu!");

		// create text layouts
		if (!createTextLayouts().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text layouts for the new key bindings menu!");

		// initialize buttons
		currentlySelectedButton = -1;
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create title buttons!");

		isPaused = false;
		firstCreation = false;

		// return success
		return {};
	}

	util::Expected<void> NewKeyBindingState::initializeButtons()
	{
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Button Save //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Save Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/buttonsSave.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickSave = [this]() -> util::Expected<bool>
		{
			// store the new key binding
			dxApp->saveKeyBindings();
			if (!dxApp->popGameState().wasSuccessful())
				return std::runtime_error("Critical error: Unable to pop new key binding state!");
			return false;	// notify stack change
		};

		// add button to the list
		menuButtons.push_back(new AnimatedButton(L"Save", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickSave, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Back Button //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Back Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/buttonsBack.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickBack = [this]() -> util::Expected<bool>
		{
			this->isPaused = true;
			if (!dxApp->popGameState().wasSuccessful())
				return std::runtime_error("Critical error: Unable to pop new key bindings state");
			return false;
		};

		// add button to the list
		menuButtons.push_back(new AnimatedButton(L"Back", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickBack, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		// set to unpaused
		this->isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::pause()
	{
		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> NewKeyBindingState::resume()
	{
		// allow mouse input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = false;

		isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> NewKeyBindingState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if (!listening)
		{
			// get active key map
			if (!isPaused)
				return handleInput(ih->activeKeyMap);
		}
		else
		{
			// got new chord
			ih->disableListening();
			setNewChord(ih->newChordBindInfo);
		}

		// return success
		return true;
	}

	util::Expected<void> NewKeyBindingState::setNewChord(std::vector<input::BindInfo>& bi)
	{
		// disable listening
		this->keySelected = true;

		// store new chord
		newChord = bi;

		// show new chord
		std::wostringstream text;
		if (newChord.empty())
			text << "New Key" << std::endl << L"not bound";
		else
		{
			text << "New Key" << std::endl;
			for (unsigned int i = 0; i < bi.size(); i++)
			{
				if (i != bi.size() - 1)
					text << dxApp->getKeyName(bi[i].getKeyCode()).get() << L" + ";
				else
					text << dxApp->getKeyName(bi[i].getKeyCode()).get();
			}
		}

		util::Expected<void> result = d2d->createTextLayoutFromWStringStream(&text, newKeyBindingFormat.Get(), (float)dxApp->getCurrentWidth(), 100, newKeyBindingLayout);
		if (!result.isValid())
			return std::runtime_error("Critical error: Unable to create text layout!");

		// change game command
		commandToChange->setChord(newChord);
		return true;
	}
	
	util::Expected<bool> NewKeyBindingState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// activate currently selected button
				if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
					return menuButtons[currentlySelectedButton]->click();
				else
					break;

			case input::ShowFPS:
				dxApp->toggleFPS();
				break;
			}
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::update(const double deltaTime)
	{
		// handle errors
		util::Expected<void> result;

		if (isPaused)
			return {};

		// capture mouse
		if (dxApp->activeMouse)
		{
			// get mouse position
			long mouseX = dxApp->getMouseX();
			long mouseY = dxApp->getMouseY();

			// check if mouse position is inside button rectangle
			bool buttonSelected = false;
			int i = 0;
			for (auto button : menuButtons)
			{
				D2D1_RECT_F rect = button->getRectangle();
				if (mouseX > rect.left && mouseX < rect.right && mouseY > rect.top && mouseY < rect.bottom)
				{
					if (currentlySelectedButton != i)
					{
						// deselect current button
						if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
							menuButtons[currentlySelectedButton]->deselect();

						// select button
						button->select();
						currentlySelectedButton = i;
						buttonSelected = true;
					}
					else
						button->select();

					buttonSelected = true;
				}
				else
					button->deselect();
				i++;
			}
			if (!buttonSelected)
				currentlySelectedButton = -1;
		}

		if (!this->keySelected)
			this->showPressKey = !this->showPressKey;

		for (auto button : menuButtons)
			button->update(deltaTime);

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::render(const double /*farSeer*/)
	{
		if (isPaused)
			return { };

		// fill rectangle
		d2d->fillRoundedRectangle(50, 50, 1870, 650, 45, 45, 1.0f, whiteBrush.Get());
		d2d->drawRoundedRectangle(50, 50, 1870, 650, 45, 45);
		d2d->printText(0, 40, titleLayout.Get());
		d2d->printText(0, 220, eventLayout.Get());
		d2d->printText(250, 380, oldKeyBindingLayout.Get());
		d2d->printText(1350, 380, newKeyBindingLayout.Get());

		if (showPressKey && !keySelected)
		{
			d2d->printText(0, 500, pressKeyLayout.Get());
			d2d->printText(0, 550, pressEscapeKeyLayout.Get());
		}

		menuButtons[0]->drawCentered(2, -300, 300);
		menuButtons[1]->drawCentered(2, 300, 300);

		// print FPS information
		d2d->printFPS();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::shutdown()
	{
		ShowCursor(false);
		this->isPaused = true;

		// tell the input handler to stop listening for user input
		dxApp->disableListeningForInput();

		// reset key selection boolean
		keySelected = false;
		
		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();

		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);
		
		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Helpers /////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> NewKeyBindingState::createTextFormats()
	{
		// handle errors
		util::Expected<void> result;

		if (firstCreation)
		{
			// title format
			result = d2d->createTextFormat(L"Lucida Handwriting", 92.0f, DWRITE_TEXT_ALIGNMENT_CENTER, titleFormat);
			if (!result.isValid())
				return result;
		}

		// game event format
		result = d2d->createTextFormat(L"Lucida Handwriting", 48.0f, DWRITE_TEXT_ALIGNMENT_CENTER, eventFormat);
		if (!result.isValid())
			return result;

		// new key binding format
		result = d2d->createTextFormat(L"Segoe Script", 48.0f, newKeyBindingFormat);
		if (!result.isValid())
			return result;

		// old key binding format
		result = d2d->createTextFormat(L"Segoe Script", 48.0f, oldKeyBindingFormat);
		if (!result.isValid())
			return result;

		if (firstCreation)
		{
			// press key format
			result = d2d->createTextFormat(L"Segoe UI", 62.0f, DWRITE_TEXT_ALIGNMENT_CENTER, pressKeyFormat);
			if (!result.isValid())
				return result;
			
			// press escapy key format
			result = d2d->createTextFormat(L"Segoe UI", 18.0f, DWRITE_TEXT_ALIGNMENT_CENTER, pressEscapeKeyFormat);
			if (!result.isValid())
				return result;
		}

		// return success
		return { };
	}
	
	util::Expected<void> NewKeyBindingState::createTextLayouts()
	{
		// handle errors
		util::Expected<void> result;

		if (firstCreation)
		{
			std::wstring title = L"Select New Key Binding";
			result = d2d->createTextLayoutFromWString(&title, titleFormat.Get(), (float)dxApp->getCurrentWidth(), 200, titleLayout);
			if (!result.isValid())
				return result;
		}

		std::wstring text = input::getGameCommandAsString(gameCommand);

		std::wostringstream eventText;
		eventText << L"for" << std::endl << text.c_str();
		result = d2d->createTextLayoutFromWStringStream(&eventText, eventFormat.Get(), (float)dxApp->getCurrentWidth(), 100, eventLayout);
		if (!result.isValid())
			return result;

		std::wostringstream oldKeyBindingText;
		oldKeyBindingText << L"Current Key" << std::endl << oldKeyBinding.c_str();
		result = d2d->createTextLayoutFromWStringStream(&oldKeyBindingText, oldKeyBindingFormat.Get(), (float)dxApp->getCurrentWidth(), 100, oldKeyBindingLayout);
		if (!result.isValid())
			return result;

		std::wostringstream newKeyBindingText;
		newKeyBindingText << L"New Key" << std::endl;
		result = d2d->createTextLayoutFromWStringStream(&newKeyBindingText, newKeyBindingFormat.Get(), (float)dxApp->getCurrentWidth(), 100, newKeyBindingLayout);
		if (!result.isValid())
			return result;

		if (firstCreation)
		{
			text = L"Press Key!";
			result = d2d->createTextLayoutFromWString(&text, pressKeyFormat.Get(), (float)dxApp->getCurrentWidth(), 100, pressKeyLayout);
			if (!result.isValid())
				return result;

			text = L"Press Escape to unbind keys!";
			result = d2d->createTextLayoutFromWString(&text, pressEscapeKeyFormat.Get(), (float)dxApp->getCurrentWidth(), 100, pressEscapeKeyLayout);
			if (!result.isValid())
				return result;
		}

		// return success
		return { };
	}
}