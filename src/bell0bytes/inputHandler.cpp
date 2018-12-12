// boost includes: for marshalling and unmarshalling of game commands
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

// bell0bytes util
#include "inputHandler.h"
#include "serviceLocator.h"

// bell0bytes graphics: for the custom mouse cursor
#include "sprites.h"

// bell0bytes core
#include "app.h"

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
	GameCommand::GameCommand(const std::wstring& name) : name(name), chord(0) {};
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
	InputHandler::InputHandler(core::DirectXApp* const dxApp, const std::wstring& keyBindingsFile) : keyBindingsFile(keyBindingsFile), dxApp(dxApp), listen(false)
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

		// delete the pointer to the keyboard and mouse class
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
			if (x.second->chord.empty())
				continue;

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
			notify(this, false);	// false: normal game input; was not listening to specially requested user input
		}
		else
		{
			if (listen)
			{
				// we are listening to specially requested user input
				newChordBindInfo.clear();

				// give the user the ability to "unbind" a key by pressing the "ESCAPE" key
				if (isPressed(VK_ESCAPE))
				{
					listen = false;			// stop listening ; produce normal input again
					notify(this, true);		// true: was listening to specially requested user input
					return;					// all done
				}

				// now loop through all possible keys and check for changes
				for (unsigned int i = 0; i < 256; i++)
				{
					// we don't care which one of the shift or ctrl keys was pressed
					if (i >= 160 && i <= 165)
						continue;

					// push the keys the user is holding down to the chord
					if (getKeyState(i) == KeyState::StillPressed)
					{
						newChordBindInfo.push_back(BindInfo(i, getKeyState(i)));
						continue;
					}
					
					// now add those keys that have been pressed
					if (kbm->currentState[i] != kbm->previousState[i]	// only listen to key state changes
						&& getKeyState(1) != KeyState::JustReleased)	// ignore when the left mouse button is released (as the menu is accessed via left mouse button click)	
						newChordBindInfo.push_back(BindInfo(i, getKeyState(i)));
				}

				// if there is a new chord, we have to make sure that we are not overwriting an already existing chord
				if (!newChordBindInfo.empty())
				{
					// check for new chord to not overwrite other commands
					bool newChord = true;
					for (auto x : keyMap)
					{
						// no chord at all -> continue
						if (x.second->chord.empty())
							continue;

						// different sizes -> can't be the same chord -> continue
						if (x.second->chord.size() != newChordBindInfo.size())
							continue;
						else
						{
							// check all key bindings
							bool allTheSame = true;
							for (unsigned int i = 0; i < newChordBindInfo.size(); i++)
							{
								if (x.second->chord[i].keyCode != newChordBindInfo[i].keyCode)
								{
									// the keys are different
									allTheSame = false;
									break;
								}
								else
								{
									// the keys are the same; check for their states
									if (x.second->chord[i].keyState != newChordBindInfo[i].keyState)
									{
										// the states are different -> check for pressed <-> released mismatch
										if (x.second->chord[i].keyState == KeyState::JustPressed && newChordBindInfo[i].keyState == KeyState::JustReleased)
										{
											// do nothing
											continue;
										}
										allTheSame = false;
										break;
									}
								}
							}
							if (allTheSame)
								newChord = false;
						}
					}

					
					if (!newChord)
						// the just pressed chord is already bound to a command -> clear and restart
						newChordBindInfo.clear();
					else
					{
						// we have a new chord ; notify if at least one of the keys was released, else: continue
						// this is necessary to capture multipe key presses, such as "CTRL" + A
						bool sendNotification = false;
						for (auto x : newChordBindInfo)
							if (getKeyState(x.keyCode) == KeyState::JustReleased)
							{
								sendNotification = true;
								listen = false;
								break;
							}
						if (sendNotification)
						{
							for (auto& x : newChordBindInfo)
							{
								if (x.keyState == KeyState::JustReleased)
									x.keyState = KeyState::JustPressed;
							}
							notify(this, true);
						}
					}
				}
			}
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

	void InputHandler::changeMouseCursorAnimationCycle(const unsigned int cycle) const
	{
		if(dxApp->activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->changeAnimation(cycle);
	}

	void InputHandler::updateMouseCursorAnimation(const double deltaTime) const
	{
		if(dxApp->activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->updateAnimation(deltaTime);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Bindings /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::saveGameCommands() const
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
		
		// open the file
		std::ifstream keyBindingsIn(keyBindingsFile, std::fstream::binary | std::fstream::in);
		boost::archive::text_iarchive ia(keyBindingsIn);
		
		// populate map with information from file
		while (!keyBindingsIn.eof())
		{
			ia >> gcs;
			ia >> unmarshalledGameCommand;
			keyMap.insert(std::pair<GameCommands, GameCommand*>(gcs, unmarshalledGameCommand));
		}

		// close the file
		keyBindingsIn.close();
	}

	void InputHandler::resetKeyStates()
	{
		this->kbm->currentState.fill(0);
		this->kbm->previousState.fill(0);
	}

	void InputHandler::enableListening()
	{
		this->resetKeyStates();
		listen = true;
	}

	void InputHandler::disableListening()
	{
		listen = false;
		this->resetKeyStates();
	}

	void InputHandler::getKeysMappedToCommand(const GameCommands gameCommand, std::vector<std::vector<BindInfo> >& map) const
	{
		std::vector<std::vector<BindInfo> > vecBI;
		for(auto x : keyMap)
			if (x.first == gameCommand)
				map.push_back(x.second->chord);
	}

	void InputHandler::getCommandsMappedToGameAction(const GameCommands gameCommand, std::vector<GameCommand*>& commands) const
	{
		for (auto x : keyMap)
			if (x.first == gameCommand)
				commands.push_back(x.second);
	}

	void InputHandler::insertNewCommand(GameCommands gameCommand, GameCommand& command)
	{
		this->keyMap.insert(std::pair<GameCommands, GameCommand*>(gameCommand, &command));
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Key Codes ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const util::Expected<std::wstring> InputHandler::getKeyName(const unsigned int keyCode) const
	{
		int scanCode = MapVirtualKey(keyCode, MAPVK_VK_TO_VSC);

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
		wchar_t keyName[256];
		if (GetKeyNameText(scanCode << 16, keyName, 256) != 0)
			return (std::wstring)keyName;
		else
		{
			HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
			if (hr == S_OK)
			{
				// might be a mouse button
				// I have no idea why they aren't caught by the above code
				switch (keyCode)
				{
				case 1:
					return (std::wstring)L"Left Mouse Button";
				case 2:
					return (std::wstring)L"Right Mouse Button";
				case 4:
					return (std::wstring)L"Middle Mouse Button";
				case 5:
					return (std::wstring)L"Extra Mouse Button 1";
				case 6:
					return (std::wstring)L"Extra Mouse Button 2";

				default:
					return (std::wstring)L"Unable to retrieve key name!";
				}

			}
			else
				return hr;
		}
	}
}