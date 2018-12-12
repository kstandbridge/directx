// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <sstream>

// bell0bytes core
#include "app.h"

// bell0bytes UI
#include "keyMapMenuState.h"
#include "newKeyBindingState.h"
#include "playState.h"
#include "buttons.h"

// bell0bytes graphics
#include "sprites.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputHandler.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	KeyMapMenuState::KeyMapMenuState(core::DirectXApp* const app, const std::wstring& name) : GameState(app, name), currentlySelectedButton(0), currentPage(0), keyBindingsPerPage(5)
	{ }

	KeyMapMenuState::~KeyMapMenuState()
	{ }

	KeyMapMenuState& KeyMapMenuState::createInstance(core::DirectXApp* const app, const std::wstring& stateName)
	{
		static KeyMapMenuState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> KeyMapMenuState::initialize()
	{
		// handle errors
		util::Expected<void> result;
		
		// add to observer list of the input handler
		dxApp->addInputHandlerObserver(this);

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

			// create text formats
			if (!createTextFormats().wasSuccessful())
				return std::runtime_error("Critical error: Unable to create text formats for the key bindings menu!");

			// create header layouts
			if (!createHeaderLayouts().wasSuccessful())
				return std::runtime_error("Critical error: Unable to create header layouts!");

			// create action text layouts
			for (unsigned int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
				if (!addTextToActionTextLayoutList((input::GameCommands)i).wasSuccessful())
					return std::runtime_error("Critical error: Unable to create header action text layouts!");
		}
		
		// create primary key bindings layout
		for (int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if(!addKeyBindingToLayoutList(0, (input::GameCommands)i).wasSuccessful())
				return std::runtime_error("Critical error: Unable to create primary key bindings layouts!");

		// create secondary key bindings layout
		for (int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if(!addKeyBindingToLayoutList(1, (input::GameCommands)i).wasSuccessful())
				return std::runtime_error("Critical error: Unable to create secondary key bindings layouts!");
		
		// initialize buttons
		currentlySelectedButton = -1;
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create title buttons!");

		firstCreation = false;
		isPaused = false;

		// return success
		return { };
	}

	
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> KeyMapMenuState::pause()
	{
		// unselect buttons
		this->currentlySelectedButton = -1;

		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> KeyMapMenuState::resume()
	{
		// unselect buttons
		this->currentlySelectedButton = -1;

		// allow mouse input
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = false;
		
		// recreate key binding layouts
		dxApp->loadKeyBindings();
		recreateLayouts();
		
		isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> KeyMapMenuState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if (!isPaused)
			if(!listening)
				return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> KeyMapMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
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
	util::Expected<void> KeyMapMenuState::update(const double deltaTime)
	{
		// handle errors
		util::Expected<void> result;

		if (isPaused)
			return {};

		// get number of game commands
		unsigned int numberOfGameCommands = input::GameCommands::nGameCommands;

		// capture mouse
		if (dxApp->activeMouse)
		{
			// get mouse position
			long mouseX = dxApp->getMouseX();
			long mouseY = dxApp->getMouseY();

			// check if mouse position is inside button rectangle
			bool buttonSelected = false;
			unsigned int i = 0;
			for (auto button : menuButtons)
			{
				D2D1_RECT_F rect = button->getRectangle();
				if (mouseX > rect.left && mouseX < rect.right && mouseY > rect.top && mouseY < rect.bottom && ((i >= currentPage*keyBindingsPerPage && i < currentPage*keyBindingsPerPage + keyBindingsPerPage) || (i >= currentPage * keyBindingsPerPage + numberOfGameCommands && i < currentPage*keyBindingsPerPage + keyBindingsPerPage + numberOfGameCommands) || i>=2*numberOfGameCommands) )
				{
					if (currentlySelectedButton != (int)i)
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

		unsigned int n = 0;
		for (unsigned int i = 0; i < currentPage + 1; i++)
			n += keyBindingsPerPage;
		if (n >= actionTextLayouts.size())
			menuButtons[2*numberOfGameCommands+2]->lock();

		if (currentPage == 0)
			menuButtons[2*numberOfGameCommands+1]->lock();

		menuButtons[0]->lock();
		menuButtons[numberOfGameCommands]->lock();
		menuButtons[1]->lock();
		menuButtons[2]->lock();

		for (auto button : menuButtons)
			button->update(deltaTime);

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> KeyMapMenuState::render(const double /*farSeer*/)
	{
		// error handling
		util::Expected<void> result;

		// get number of game commands to loop through all the commands and buttons
		unsigned int numberOfGameCommands = input::GameCommands::nGameCommands;
		
		if (!isPaused)
		{
			// fill rectangle
			d2d->fillRoundedRectangle(50, 50, 1870, 650, 45, 45, 1.0f, whiteBrush.Get());
			d2d->drawRoundedRectangle(50, 50, 1870, 650, 45, 45);
			d2d->printText(0, 50, titleLayout.Get());

			// print header
			d2d->printText((float)165, (float)150, headerTextLayouts[0].Get());
			d2d->printText((float)765, (float)150, headerTextLayouts[1].Get());
			d2d->printText((float)1455, (float)150, headerTextLayouts[2].Get());

			// draw map texts
			unsigned int offset = 0;
			for (unsigned int i = currentPage * keyBindingsPerPage; i < currentPage*keyBindingsPerPage + keyBindingsPerPage; i++)
			{
				if (i < 0 || i >= actionTextLayouts.size())
					break;

				offset = i % keyBindingsPerPage;

				// print action texts
				d2d->printText((float)65, (float)225 + offset * 75, actionTextLayouts[i].Get());

				// print primary key bindings
				d2d->printText((float)765, (float)225 + offset * 75, keyBindings1TextLayouts[i].Get());

				// draw primary key binding gamepad button
				menuButtons[i]->draw(1, (float)700, (float)262 + offset * 75);

				// print secondary key bindings
				d2d->printText((float)1365, (float)225 + offset * 75, keyBindings2TextLayouts[i].Get());

				// draw secondary key binding gamepad buttons
				menuButtons[i + numberOfGameCommands]->draw(1, (float)1300, (float)262 + offset * 75);

				// draw horizontal lines
				d2d->drawRectangle(50, (float)225 + (offset + 1) * 75, 1870, (float)225 + (offset + 1) * 75);
			}
		}

		menuButtons[2*numberOfGameCommands]->drawCentered(2, 300, 300);
		
		if (!isPaused)
		{
			menuButtons[2*numberOfGameCommands+1]->drawCentered(1, -500, 150);
			menuButtons[2*numberOfGameCommands+2]->drawCentered(1, 500, 150);
		}

		// print FPS information
		d2d->printFPS();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> KeyMapMenuState::shutdown()
	{
		this->currentlySelectedButton = -1;

		ShowCursor(false);
		this->isPaused = true;

		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();

		// release and clear layouts
		releaseAndClearLayouts();

		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Helpers /////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	
	// text formats and layouts
	util::Expected<void> KeyMapMenuState::addKeyBindingToLayoutList(const unsigned int i, input::GameCommands gameCommand)
	{
		if (i < 0 || i > 1)
			return std::runtime_error("Critical error: can't add key binding to layout list!");

		// create text and layout
		std::wostringstream keyBindingText;
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> textLayout;

		// get keys that are actually mapped to the specified command
		std::vector<std::vector<input::BindInfo> > vecBI;
		dxApp->getKeysMappedToCommand(gameCommand, vecBI);
		
		if (i == 0)
		{
			if (vecBI.empty())
				keyBindingText << "not bound";
			else
			{
				for (unsigned int j=0; j<vecBI[i].size(); j++)
				{
					util::Expected<std::wstring> keyName(dxApp->getKeyName(vecBI[i][j].getKeyCode()));
					if (keyName.isValid())
					{
						if (j != vecBI[i].size() - 1)
							keyBindingText << keyName.get() << " + ";
						else
							keyBindingText << keyName.get();
					}
					else
						return std::runtime_error("Critical error: Failed to get key name!");
				}
				if (keyBindingText.str() == L"")
					keyBindingText <<  L"not bound";
			}
		}
		else
		{
			if (vecBI.size() != 2)
				keyBindingText << "not bound";
			else
			{
				for (unsigned int j = 0; j<vecBI[i].size(); j++)
				{
					util::Expected<std::wstring> keyName(dxApp->getKeyName(vecBI[i][j].getKeyCode()));
					if (keyName.isValid())
					{
						if (j != vecBI[i].size() - 1)
							keyBindingText << keyName.get() << " + ";
						else
							keyBindingText << keyName.get();
					}
					else
						return std::runtime_error("Critical error: Failed to get key name!");
				}
				if (keyBindingText.str() == L"")
					keyBindingText << "not bound";
			}
		}

		util::Expected<void> result = d2d->createTextLayoutFromWStringStream(&keyBindingText, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, textLayout);
		if (!result.isValid())
			return result;

		if (i == 0)
		{
			keyBindings1TextLayouts.push_back(textLayout);
			keyBindings1Texts.push_back(keyBindingText.str());
		}
		else
		{
			keyBindings2TextLayouts.push_back(textLayout);
			keyBindings2Texts.push_back(keyBindingText.str());
		}

		// return success
		return { };
	}
	util::Expected<void> KeyMapMenuState::addTextToActionTextLayoutList(input::GameCommands gameCommand)
	{
		std::wstring text = input::getGameCommandAsString(gameCommand);
				
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> textLayout;
		util::Expected<void> result = d2d->createTextLayoutFromWString(&text, textFormat.Get(), (float)dxApp->getCurrentWidth(), 100, textLayout);
		if (!result.isValid())
			return result;
		actionTextLayouts.push_back(textLayout);

		// return success
		return { };
	}
	util::Expected<void> KeyMapMenuState::createTextFormats()
	{
		util::Expected<void> result;

		result = d2d->createTextFormat(L"Lucida Handwriting", 92.0f, DWRITE_TEXT_ALIGNMENT_CENTER, titleFormat);
		if (!result.isValid())
			return result;

		result = d2d->createTextFormat(L"Segoe Script", 32.0f, headerFormat);
		if (!result.isValid())
			return result;

		result = d2d->createTextFormat(L"Segoe Script", 48.0f, textFormat);
		if (!result.isValid())
			return result;

		// return success
		return {};
	}
	util::Expected<void> KeyMapMenuState::createHeaderLayouts()
	{
		util::Expected<void> result;

		// create title layout
		std::wstring title = L"Key Bindings";
		result = d2d->createTextLayoutFromWString(&title, titleFormat.Get(), (float)dxApp->getCurrentWidth(), 100, titleLayout);
		if (!result.isValid())
			return result;

		// create header text layout
		std::wstring header = L"Action";
		Microsoft::WRL::ComPtr<IDWriteTextLayout4> headerLayout;
		result = d2d->createTextLayoutFromWString(&header, headerFormat.Get(), 500, 100, headerLayout);
		if (!result.isValid())
			return result;
		headerTextLayouts.push_back(headerLayout);

		header = L"Primary Key Binding";
		result = d2d->createTextLayoutFromWString(&header, headerFormat.Get(), (float)dxApp->getCurrentWidth(), 100, headerLayout);
		if (!result.isValid())
			return result;
		headerTextLayouts.push_back(headerLayout);

		header = L"Secondary Key Binding";
		result = d2d->createTextLayoutFromWString(&header, headerFormat.Get(), (float)dxApp->getCurrentWidth(), 100, headerLayout);
		if (!result.isValid())
			return result;
		headerTextLayouts.push_back(headerLayout);

		// return success
		return {};
	}
	util::Expected<void> KeyMapMenuState::recreateLayouts()
	{
		// release layouts
		releaseAndClearLayouts();

		// create primary key bindings layout
		for (int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if (!addKeyBindingToLayoutList(0, (input::GameCommands)i).wasSuccessful())
				return std::runtime_error("Critical error: Unable to create primary key bindings layouts!");

		// create secondary key bindings layout
		for (int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if (!addKeyBindingToLayoutList(1, (input::GameCommands)i).wasSuccessful())
				return std::runtime_error("Critical error: Unable to create secondary key bindings layouts!");

		// return success
		return {};
	}
	void KeyMapMenuState::releaseAndClearLayouts()
	{
		for (auto x : keyBindings1TextLayouts)
			x.ReleaseAndGetAddressOf();
		keyBindings1TextLayouts.clear();

		for (auto x : keyBindings2TextLayouts)
			x.ReleaseAndGetAddressOf();
		keyBindings2TextLayouts.clear();

		keyBindings1Texts.clear();
		keyBindings2Texts.clear();
	}
	
	// buttons
	util::Expected<void> KeyMapMenuState::initializeButtons()
	{
		// create gamepad buttons for primary key bindings
		for (unsigned int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if (!createGamepadButton(i).wasSuccessful())
				return std::runtime_error("Critical error: unable to create button!");
		
		// create gamepad buttons for secondary key bindings
		for (unsigned int i = input::GameCommands::Select; i < input::GameCommands::nGameCommands; i++)
			if (!createGamepadButton(i).wasSuccessful())
				return std::runtime_error("Critical error: unable to create button!");

		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

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
			if(!dxApp->popGameState().wasSuccessful())
				return std::runtime_error("Critical error: Unable to pop the key map menu!");
			return false;
		};

		// add button to the list
		menuButtons.push_back(new AnimatedButton(L"Back", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickBack, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Left Arrow Button ////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Left Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Locked";
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
		try { animations = new graphics::AnimationData(d2d, L"Art/buttonsLeftArrow.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickLeft = [this]
		{
			if (currentPage > 0)
				this->currentPage--;
			return true;
		};

		// add button to the list
		menuButtons.push_back(new AnimatedButton(L"Left Arrow", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickLeft, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Right Arrow Button ////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Right Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Locked";
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
		try { animations = new graphics::AnimationData(d2d, L"Art/buttonsRightArrow.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickRight = [this]
		{
			unsigned int n = 0;
			for (unsigned int i = 0; i < currentPage + 1; i++)
				n += keyBindingsPerPage;
			if (n < actionTextLayouts.size())
				currentPage++;
			return true;
		};

		// add button to the list
		menuButtons.push_back(new AnimatedButton(L"Right Arrow", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickRight, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		// set to unpaused
		this->isPaused = false;

		// return success
		return {};
	}
	util::Expected<void> KeyMapMenuState::createGamepadButton(unsigned int i)
	{
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;
		// cycle
		std::wstring text = L"Gamepad ";
		text + std::to_wstring(i) + L" Normal";
		cycle.name = text.c_str();
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		text = L"Gamepad ";
		text + std::to_wstring(i) + L" Hover";
		cycle.name = text.c_str();
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		text = L"Gamepad ";
		text + std::to_wstring(i) + L" Click";
		cycle.name = text.c_str();
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		text = L"Gamepad ";
		text + std::to_wstring(i) + L" Locked";
		cycle.name = text.c_str();
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create button animations
		try { animations = new graphics::AnimationData(d2d, L"Art/buttonsGamepad.png", animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickGamepad = [this]
		{
			return this->changeKeyBinding();
		};

		// add button to the list
		text = L"Gamepad Button ";
		text + std::to_wstring(i);
		menuButtons.push_back(new AnimatedButton(text.c_str(), new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickGamepad, 4));

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		// return success
		return { };
	}

	// change key bindings
	util::Expected<bool> KeyMapMenuState::changeKeyBinding()
	{
		UI::NewKeyBindingState* bindNewKey = &UI::NewKeyBindingState::createInstance(dxApp, L"New Key Binding");

		this->isPaused = true;
		
		// get currently selected button modulo number of game commands
		// 0-numberOfGameCommands-1: primary key binding
		// numberOfGameCommands - 2*numberOfGameCommands - 1: secondary key binding
		unsigned int numberOfGameCommands = input::GameCommands::nGameCommands;
		bool primary = currentlySelectedButton > (int)numberOfGameCommands ? false : true;
		unsigned int gameCommand = currentlySelectedButton % numberOfGameCommands;

		std::wstring text = input::getGameCommandAsString((input::GameCommands)gameCommand);
		
		// get game commands associated with the selected game action
		std::vector<input::GameCommand*> commands;
		dxApp->getCommandsMappedToGameAction((input::GameCommands)gameCommand, commands);
		
		// get command (primary or secondary)
		if (primary)
		{
			// primary key binding
			if (!commands.empty())
			{
				// the command already exists -> change the associated chord
				bindNewKey->setCommandToChange(commands[0]);
				bindNewKey->setOldKeyBindingString(keyBindings1Texts[gameCommand]);
				bindNewKey->setGameCommand((input::GameCommands)gameCommand);
				dxApp->pushGameState(bindNewKey);
			}
			else
			{
				// the command does not exist -> create a new one
				input::GameCommand* gc = new input::GameCommand(text);
				dxApp->addNewCommand((input::GameCommands)gameCommand, *gc);
				bindNewKey->setCommandToChange(gc);
				bindNewKey->setOldKeyBindingString(L"not bound");
				bindNewKey->setGameCommand((input::GameCommands)gameCommand);
				dxApp->pushGameState(bindNewKey);
			}
		}
		else
		{
			// secondary key binding
			if (commands.size() > 1)
			{
				// the command does exist -> change it
				bindNewKey->setCommandToChange(commands[1]);
				bindNewKey->setOldKeyBindingString(keyBindings1Texts[gameCommand]);
				bindNewKey->setGameCommand((input::GameCommands)gameCommand);
				dxApp->pushGameState(bindNewKey);
			}
			else
			{
				// the command does not exist -> create it
				input::GameCommand* gc = new input::GameCommand(text);
				dxApp->addNewCommand((input::GameCommands)gameCommand, *gc);
				bindNewKey->setCommandToChange(gc);
				bindNewKey->setOldKeyBindingString(L"not bound");
				bindNewKey->setGameCommand((input::GameCommands)gameCommand);
				dxApp->pushGameState(bindNewKey);
			}
		}
		return true;
	}
}