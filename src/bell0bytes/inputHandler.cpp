#include "inputHandler.h"
#include "serviceLocator.h"

namespace input
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	BindInfo::BindInfo() : keyCode(0), keyState(KeyState::JustReleased) {};
	BindInfo::BindInfo(const unsigned int keyCode, const KeyState keyState) : keyCode(keyCode), keyState(keyState) {};

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
	
	InputHandler::InputHandler() 
	{
		keyboardStateCurrent.fill(0);
		keyboardStateCurrent.fill(0);

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was successfully initialized.");
	};

	InputHandler::~InputHandler()
	{
		// clear key map
		for (auto x : keyMap)
			delete x.second;
		keyMap.clear();
		
		// clear active key map
		for (auto x : activeKeyMap)
			delete x.second;
		activeKeyMap.clear();

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was shutdown successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Polling ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::acquireInput()
	{
		// get keyboard state
		getKeyboardState();

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
			for(auto y : x.second->chord)
			{
				if (getKeyboardKeyState(y.keyCode) != y.keyState)
				{
					isActive = false;
					break;
				}
			}
			if(isActive)
				activeKeyMap.insert(std::pair<GameCommands, GameCommand*>(x.first, x.second));
		}

		// delegate to the UI

		// delegate to the game
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// KEYBOARD /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::getKeyboardState()
	{
		// store the old keyboard state
		keyboardStatePrevious = keyboardStateCurrent;

		// read the current keyboard state
		for (int i = 0; i < 256; i++)
			keyboardStateCurrent[i] = isPressed(i);
	}

	const KeyState InputHandler::getKeyboardKeyState(const unsigned int keyCode) const
	{
		if (keyboardStatePrevious[keyCode] == 1)
			if (keyboardStateCurrent[keyCode] == 1)
				return KeyState::StillPressed;
			else
				return KeyState::JustReleased;
		else
			if (keyboardStateCurrent[keyCode] == 1)
				return KeyState::JustPressed;
			else
				return KeyState::StillReleased;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Bindings /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	
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