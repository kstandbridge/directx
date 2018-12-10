#include "inputHandler.h"
#include "serviceLocator.h"
#include <array>
#include <assert.h>
#include "sprites.h"
#include "app.h"

// boost includes
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace input
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////// Constructors and Destructors //////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	// BindInfo
	BindInfo::BindInfo() : keyCode(0), keyState(KeyState::JustReleased) {};
	BindInfo::BindInfo(const unsigned int keyCode, const KeyState keyState) : keyCode(keyCode), keyState(keyState) {};

	// GameCommand
	GameCommand::GameCommand() : name(L""), chord(0) {};
	GameCommand::GameCommand(const std::wstring& name, const unsigned int keyCode, const KeyState keyState) : name(name)
	{
		chord.push_back(BindInfo(keyCode, keyState));
	}
	GameCommand::GameCommand(const std::wstring& name, const BindInfo& bi) : name(name)
	{
		chord.push_back(bi);
	};
	GameCommand::GameCommand(const std::wstring& name, const std::vector<BindInfo>& chord) : name(name), chord(chord) {};
	
	// Keyboard and Mouse
	KeyboardAndMouse::KeyboardAndMouse() : mouseX(0), mouseY(0)
	{
		this->currentState.fill(false);
		this->previousState.fill(false);
		this->mouseCursor = nullptr;
	}

	KeyboardAndMouse::KeyboardAndMouse(graphics::AnimatedSprite* const mouseCursor) : KeyboardAndMouse()
	{
		this->mouseCursor = mouseCursor;
	}

	KeyboardAndMouse::~KeyboardAndMouse()
	{
		if(mouseCursor)
			delete this->mouseCursor;
	}

	// Input Handler
	InputHandler::InputHandler(core::DirectXApp* const dxApp, const std::wstring& keyBindingsFile) : keyBindingsFile(keyBindingsFile), dxApp(dxApp)
	{
		// initialize keyboard and mouse
		kbm = new KeyboardAndMouse();
		dxApp->activeMouse = true;
		dxApp->activeKeyboard = true;

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was successfully initialized.");
	};

	InputHandler::~InputHandler()
	{
		// clear active key map
		activeKeyMap.clear();

		// clear key map
		for (auto x : keyMap)
		{
			if(x.second != nullptr)
				delete x.second;
		}
		keyMap.clear();


		delete kbm;
		
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was shutdown successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Initialization ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Polling ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::acquireInput()
	{
		if(dxApp->activeKeyboard || dxApp->activeMouse)
			// get keyboard and mouse state
			getKeyboardAndMouseState();

		// update the key maps
		update();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Update ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::update()
	{
		// clear out any active bindings from the last frame
		bool isActive = false;
		activeKeyMap.clear();
		
		// loop through the map of all possible actions and find the active key bindings
		for (auto x : keyMap)
		{
			// test chord
			isActive = true;
			for (auto y : x.second->chord)
			{
				if (getKeyState(y.keyCode) != y.keyState)
				{
					isActive = false;
					break;
				}
			}
			if (isActive)
				activeKeyMap.insert(std::pair<GameCommands, GameCommand&>(x.first, *x.second));
		}

		// if there is an active key map
		if (!activeKeyMap.empty())
		{
			// notify the currently active game states to handle the input
			notify(activeKeyMap);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Keyboard and Mouse ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::getKeyboardAndMouseState()
	{
		if (dxApp->activeKeyboard || dxApp->activeMouse)
		{
			// store the old keyboard state
			kbm->previousState = kbm->currentState;

			// read the current keyboard state
			for (int i = 0; i < 256; i++)
				kbm->currentState[i] = isPressed(i);
		}

		if (dxApp->activeMouse)
		{
			// get the position of the mouse cursor
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			kbm->mouseX = cursorPos.x;
			kbm->mouseY = cursorPos.y;

			// set the cursor position
			if (kbm->mouseCursor != nullptr)
				kbm->mouseCursor->setPosition((float)kbm->mouseX, (float)kbm->mouseY);
		}
	}

	const KeyState InputHandler::getKeyState(const unsigned int keyCode) const
	{
		if (kbm->previousState[keyCode] == 1)
			if (kbm->currentState[keyCode] == 1)
				return KeyState::StillPressed;
			else
				return KeyState::JustReleased;
		else
			if (kbm->currentState[keyCode] == 1)
				return KeyState::JustPressed;
			else
				return KeyState::StillReleased;
	}

	void InputHandler::drawMouseCursor() const
	{
		if(dxApp->activeMouse)
			if(kbm->mouseCursor != nullptr)
				kbm->mouseCursor->draw();
	}

	void InputHandler::changeMouseCursorAnimationCycle(const unsigned int cycle)
	{
		if(dxApp->activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->changeAnimation(cycle);
	}

	void InputHandler::updateMouseCursorAnimation(const double deltaTime)
	{
		if(dxApp->activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->updateAnimation(deltaTime);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Bindings /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::saveGameCommands()
	{
		std::ofstream keyBindingsOut(this->keyBindingsFile, std::ios::out);
		boost::archive::text_oarchive oa(keyBindingsOut);

		for (auto gameCommand : keyMap)
		{
			oa << gameCommand.first;
			oa << gameCommand.second;
		}

		keyBindingsOut.close();
	}

	void InputHandler::loadGameCommands()
	{
		// create default key bindings if file does not yet exist
		if (GetFileAttributes(keyBindingsFile.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			setDefaultKeyMap();
			saveGameCommands();
		}

		// clear map
		keyMap.clear();

		// load data
		GameCommands gcs;
		GameCommand* gameCommand = nullptr;

		// open the file
		std::ifstream keyBindingsIn(keyBindingsFile);	
		boost::archive::text_iarchive ia(keyBindingsIn);

		// populate map with information from file
		while (!keyBindingsIn.eof())
		{
			ia >> gcs;
			ia >> gameCommand;
			keyMap.insert(std::pair<GameCommands, GameCommand*>(gcs, gameCommand));
		}

		gameCommand = nullptr;

		// close the file
		keyBindingsIn.close();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Key Codes ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<std::wstring> InputHandler::getKeyName(const unsigned int keyCode)
	{
		unsigned int scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);

		// the MapVirtualKey method strips the extended bit for some keys
		// thus we have to add them again
		switch (keyCode)
		{
			case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
			case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:	
			case VK_INSERT: case VK_DELETE: case VK_DIVIDE: case VK_NUMLOCK:
				scanCode |= 0x100;
				break;
		}

		// now get the key name
		LPWSTR keyName{};
		if (GetKeyNameText(scanCode << 16, keyName, 50 != 0))
			return keyName;
		else
			return std::runtime_error("Critical error: Unable to retrieve key name!");
	}
}