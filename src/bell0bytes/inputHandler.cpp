// boost includes: for marshalling and unmarshalling of game commands
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

// XInput together with DirectInput
#include <wbemidl.h>
#include <oleauto.h>

// Lua and Sol
#include <sol.hpp>
#pragma comment(lib, "liblua53.a")

// include class header
#include "inputHandler.h"

// bell0bytes util
#include "serviceLocator.h"
#include "stringConverter.h"

// bell0bytes core
#include "app.h"
#include "states.h"

// bell0bytes file system
#include "fileSystemComponent.h"

// bell0bytes graphics: for the custom mouse cursor
#include "sprites.h"

// bell0bytes mathematics
#include "vectors.h"

// bell0bytes input
#include "gameCommands.h"

// MSDN macros
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }
#define SAFE_DELETE(a) if( (a) != NULL ) delete (a); (a) = NULL;

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

		if (!mouseCursor)
			throw std::runtime_error("Critical error: null pointer passed as mouse cursor!");
	}
	KeyboardAndMouse::~KeyboardAndMouse()
	{
		if(mouseCursor)
			delete this->mouseCursor;
	}

	// Joystick
	Joystick::Joystick(LPDIRECTINPUTDEVICE8 const gc) : dev(gc)
	{ 
		// get game controller name
		DIDEVICEINSTANCE deviceInfo;
		deviceInfo.dwSize = sizeof(DIDEVICEINSTANCE);
		dev->GetDeviceInfo(&deviceInfo);
		name = deviceInfo.tszInstanceName;

		// get game controller capabilities
		DIDEVCAPS capabilities;
		capabilities.dwSize = sizeof(DIDEVCAPS);
		dev->GetCapabilities(&capabilities);

		numberOfAxes = capabilities.dwAxes;
		numberOfPOVs = capabilities.dwPOVs;
		numberOfButtons = capabilities.dwButtons;

		// clean joystick states
		ZeroMemory(&previousState, sizeof(DIJOYSTATE));
		ZeroMemory(&currentState, sizeof(DIJOYSTATE));
	}
	Joystick::~Joystick()
	{ }

	// Gamepad
	Gamepad::Gamepad(const unsigned int playerID) : playerID(playerID)
	{
		ZeroMemory(&currentState, sizeof(XINPUT_STATE));
		ZeroMemory(&previousState, sizeof(XINPUT_STATE));
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

		// create the axes vectors
		thumbStickLeft = new maths::Vector2F();
		thumbStickRight = new maths::Vector2F();
	}
	Gamepad::~Gamepad()
	{
		delete thumbStickLeft;
		delete thumbStickRight;
	}

	// Input Handler
	InputHandler::InputHandler(core::DirectXApp& dxApp, const HINSTANCE& hInstance, const HWND& appWindow, const std::wstring& keyBindingsFileKeyboard, const std::wstring& keyBindingsFileJoystick, const std::wstring& keyBindingsFileGamepad) : keyBindingsFileKeyboard(keyBindingsFileKeyboard), keyBindingsFileJoystick(keyBindingsFileJoystick), keyBindingsFileGamepad(keyBindingsFileGamepad), dxApp(dxApp), listen(false), nGamepads(0), nPlayers(1)
	{
		// read configuration file
		readConfigFile();

		// initialize keyboard and mouse
		kbm = new KeyboardAndMouse();
		activeMouse = true;
		activeKeyboard = true;

		// initialize XInput gamepads
		bool foundGamepad = false;
		if (activeGamepad)
		{
			// initialize XInput gamepads
			if (initializeXInputGamepads())
				foundGamepad = true;
			else
				activeGamepad = false;
		}

		if (activeJoystick || (!foundGamepad && !activeGamepad))
		{
			// initialize DirectInput 8
			util::Expected<void> result = initializeDirectInput(hInstance, appWindow);
			if (!result.isValid())
				throw result;
		}

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was successfully initialized.");
	};
	InputHandler::~InputHandler()
	{
		// clear active key map
		activeKeyMap.clear();

		// clear key maps
		if (!activeJoystick && !activeGamepad)
		{
			// clear keyboard key map
			for (auto x : keyMapKeyboard)
			{
				if (x.second != nullptr)
					delete x.second;
			}
			keyMapKeyboard.clear();
		}
		else if (activeJoystick)
		{
			// clear joystick key map
			for (auto x : keyMapJoystick)
			{
				if (x.second != nullptr)
					delete x.second;
			}
			keyMapJoystick.clear();
		}
		else if (activeGamepad)
		{
			// clear gamepad key map
			for (auto x : keyMapGamepad)
			{
				if (x.second != nullptr)
					delete x.second;
			}
			keyMapGamepad.clear();
		}
		
		// delete the pointer to the keyboard and mouse class
		delete kbm;

		// release controllers
		for (auto controller : gameControllersDI)
		{	
			controller->Unacquire();
			controller->Release();
		}
		gameControllersDI.clear();

		// release gamepads
		for (auto x : gameControllersXI)
		{
			if (x)
				delete x;
		}

		// release the main DirectInput device
		SAFE_RELEASE(dev);
				
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The input handler was shutdown successfully.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Initialization ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Polling ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> InputHandler::acquireInput()
	{
		if(activeKeyboard || activeMouse)
			// get keyboard and mouse state
			getKeyboardAndMouseState();

		if (activeGamepad)
			// poll each gamepad - we currently only support one player
			gamepad->poll();

		if (activeJoystick)
		{	
			// handle errors
			HRESULT hr;

			// poll the joystick
			hr = joystick->poll();

			if (hr == E_FAIL)
				return std::runtime_error("Critical error: Unable to poll the joystick device!");
		}

		// update the key maps
		return update();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////// Update ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> InputHandler::update()
	{
		// if the gamepad is active, check for simple gamepad movement
		if (activeGamepad)
		{
			if (gamepad->previousState.dwPacketNumber != gamepad->currentState.dwPacketNumber)
			{
				// put a message on the event queue for each state on the state queue
				std::deque<core::GameState*> states;
				dxApp.getActiveStates(states);
				for (std::deque<core::GameState*>::reverse_iterator it = states.rbegin(); it != states.rend(); it++)
				{
					if (!(*it)->isPaused)
					{
						core::DepescheDestination* destination = *it;
						core::Depesche depesche(*this, *destination, core::DepescheTypes::Gamepad, false);
						dxApp.addMessage(depesche);
					}
				}
			}
		}

		// clear out any active bindings from the last frame
		bool isActive = false;
		activeKeyMap.clear();

		std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;

		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;
		
		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to find input device!");

		// loop through the map of all possible actions and find the active key bindings
		for (auto x : *keyMap)
		{
			if (x.second->chord.empty())
				continue;

			// test chord
			isActive = true;
			for (auto y : x.second->chord)
			{
				if (y.keyCode >= 0 && y.keyCode < joystickBegin)
				{
					// this is a keyboard or mouse chord
					if (getKeyState(y.keyCode) != y.keyState)
					{
						isActive = false;
						break;
					}
				}
				else if (y.keyCode >= joystickBegin && y.keyCode < gamepadBegin && activeJoystick)
				{
					// this is a joystick chord
					JoystickButtons jb = (JoystickButtons)(y.keyCode % joystickBegin);
					if (joystick->getButtonState(jb) != y.keyState)
					{
						isActive = false;
						break;
					}
				}
				else
				{
					if (y.keyCode == 18000)
					{
						// left trigger
						if (gamepad->getDigitalTriggerState(0) != y.keyState)
						{
							isActive = false;
							break;
						}
					}
					else if (y.keyCode == 18001)
					{
						// left trigger
						if (gamepad->getDigitalTriggerState(1) != y.keyState)
						{
							isActive = false;
							break;
						}
					}
					else
					{
						// this is a gamepad chord
						if (gamepad->getButtonState((WORD)(y.keyCode - gamepadBegin)) != y.keyState)
						{
							isActive = false;
							break;
						}
					}
				}
			}
			
			if (isActive)
				activeKeyMap.insert(std::pair<GameCommands, GameCommand&>(x.first, *x.second));
		}

		// if there is an active key map
		if (!activeKeyMap.empty())
		{
			// put a message on the event queue for each state on the state queue
			std::deque<core::GameState*> states;
			dxApp.getActiveStates(states);
			for (std::deque<core::GameState*>::reverse_iterator it = states.rbegin(); it != states.rend(); it++)
			{
				if (!(*it)->isPaused)
				{
					core::DepescheDestination* destination = *it;
					core::Depesche depesche(*this, *destination, core::DepescheTypes::ActiveKeyMap, false);
					dxApp.addMessage(depesche);
				}
			}
		}
		else
		{
			bool firstTime = true;
			if (firstTime)
			{
				// reset left mouse button
				kbm->currentState[1] = 0;
				kbm->previousState[1] = 0;
				firstTime = false;
			}

			if (listen)
			{
				// we are listening to specially requested user input
				newChordBindInfo.clear();

				// give the user the ability to "unbind" a key by pressing the "ESCAPE" key
				if (isPressed(VK_ESCAPE))
				{
					listen = false;			// stop listening ; produce normal input again
					
					// put a message on the event queue for each state on the state queue
					std::deque<core::GameState*> states;
					dxApp.getActiveStates(states);
					for (std::deque<core::GameState*>::reverse_iterator it = states.rbegin(); it != states.rend(); it++)
					{
						if (!(*it)->isPaused)
						{
							core::DepescheDestination* destination = (*it);
							bool doListen = true;
							core::Depesche depesche(*this, *destination, core::DepescheTypes::ActiveKeyMap, &doListen);
							dxApp.addMessage(depesche);
						}
					}
					return {};				// all done
				}

				// now loop through all possible keys and check for changes
				for (unsigned int i = 0; i < gamepadBegin; i++)
				{
					if (i >= 0 && i < joystickBegin)
					{
						// this is a keyboard or mouse key

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
						if (kbm->currentState[i] != kbm->previousState[i])	// only listen to key state changes
							//&& getKeyState(1) != KeyState::JustReleased)	// ignore when the left mouse button is released (as the menu is accessed via left mouse button click)	
							newChordBindInfo.push_back(BindInfo(i, getKeyState(i)));
					}
					else if(i >= joystickBegin && i < gamepadBegin && activeJoystick)
					{
						// this is a joystick key
						JoystickButtons button = (JoystickButtons) (i % joystickBegin);

						if (button == JoystickButtons::EndButtons || button == JoystickButtons::EndPOV)
							continue;

						if (joystick->getButtonState(button) == KeyState::StillPressed)
						{
							newChordBindInfo.push_back(BindInfo(i, joystick->getButtonState(button)));
							continue;
						}
						
						// now add those keys that have been pressed
						if (joystick->wasPressed(button) != joystick->isPressed(button))
							newChordBindInfo.push_back(BindInfo(i, joystick->getButtonState(button)));
					}
					else if (activeGamepad)
					{
						// check trigger buttons
						if (gamepad->getDigitalTriggerState(0) == KeyState::StillPressed)
							newChordBindInfo.push_back(BindInfo(18000, gamepad->getButtonState(KeyState::StillPressed)));
						if (gamepad->getDigitalTriggerState(1) == KeyState::StillPressed)
							newChordBindInfo.push_back(BindInfo(18001, gamepad->getButtonState(KeyState::StillPressed)));
						
						if (gamepad->getDigitalTriggerState(0) == KeyState::JustPressed)
							newChordBindInfo.push_back(BindInfo(18000, gamepad->getButtonState(KeyState::JustPressed)));
						if (gamepad->getDigitalTriggerState(1) == KeyState::JustPressed)
							newChordBindInfo.push_back(BindInfo(18001, gamepad->getButtonState(KeyState::JustPressed)));

						if (gamepad->getDigitalTriggerState(0) == KeyState::JustReleased)
							newChordBindInfo.push_back(BindInfo(18000, gamepad->getButtonState(KeyState::JustReleased)));
						if (gamepad->getDigitalTriggerState(1) == KeyState::JustReleased)
							newChordBindInfo.push_back(BindInfo(18001, gamepad->getButtonState(KeyState::JustReleased)));

						// this is a gamepad button
						for (WORD j = 1; j <= 16384; j = j << 1)
						{
							if (gamepad->getButtonState(j) == KeyState::StillPressed)
							{
								newChordBindInfo.push_back(BindInfo(j+gamepadBegin, gamepad->getButtonState(j)));
								continue;
							}

							// now add those keys that have been pressed
							if (gamepad->wasPressed(j) != gamepad->isPressed(j))
								newChordBindInfo.push_back(BindInfo(j+gamepadBegin, gamepad->getButtonState(j)));
						}
						break;
					}
				}

				// if there is a new chord, we have to make sure that we are not overwriting an already existing chord
				if (!newChordBindInfo.empty())
				{
					// check for new chord to not overwrite other commands
					bool newChord = true;
					for (auto x : *keyMap)
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
						{
							if (x.keyCode >= 0 && x.keyCode < joystickBegin)
							{
								// this is a keyboard or mouse button
								if (getKeyState(x.keyCode) == KeyState::JustReleased)
								{
									sendNotification = true;
									listen = false;
									break;
								}
							}
							else if (x.keyCode >= joystickBegin && x.keyCode < gamepadBegin)
							{
								// this is a joystick button
								JoystickButtons button = (JoystickButtons)(x.keyCode % joystickBegin);
								if (joystick->getButtonState(button) == KeyState::JustReleased)
								{
									sendNotification = true;
									listen = false;
									break;
								}
							}
							else
							{
								if (x.keyCode == 18000)
								{
									if (gamepad->getDigitalTriggerState(0) == KeyState::JustReleased)
									{
										sendNotification = true;
										listen = false;
										break;
									}
								}

								if (x.keyCode == 18001)
								{
									if (gamepad->getDigitalTriggerState(1) == KeyState::JustReleased)
									{
										sendNotification = true;
										listen = false;
										break;
									}
								}
								
								if (gamepad->getButtonState((WORD)(x.keyCode - gamepadBegin)) == KeyState::JustReleased)
								{
									sendNotification = true;
									listen = false;
									break;
								}
							}
						}

						if (sendNotification)
						{
							for (auto& x : newChordBindInfo)
							{
								if (x.keyState == KeyState::JustReleased)
									x.keyState = KeyState::JustPressed;
							}

							// put a message on the event queue for each state on the state queue
							std::deque<core::GameState*> states;
							dxApp.getActiveStates(states);
							for (std::deque<core::GameState*>::reverse_iterator it = states.rbegin(); it != states.rend(); it++)
							{
								if (!(*it)->isPaused)
								{
									core::DepescheDestination* destination = (*it);
									bool doListen = true;
									core::Depesche depesche(*this, *destination, core::DepescheTypes::ActiveKeyMap, &doListen);
									dxApp.addMessage(depesche);
								}
							}
						}
					}
				}
			}
		}

		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Keyboard and Mouse ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
		void InputHandler::getKeyboardAndMouseState()
	{
		if (activeKeyboard || activeMouse)
		{
			// store the old keyboard state
			kbm->previousState = kbm->currentState;

			// read the current keyboard state
			for (int i = 0; i < 256; i++)
				kbm->currentState[i] = isPressed(i);
		}

		if (activeMouse)
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
		if(activeMouse)
			if(kbm->mouseCursor != nullptr)
				kbm->mouseCursor->draw();
	}
	void InputHandler::changeMouseCursorAnimationCycle(const unsigned int cycle) const
	{
		if(activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->changeAnimation(cycle);
	}
	void InputHandler::updateMouseCursorAnimation(const double deltaTime) const
	{
		if(activeMouse)
			if (kbm->mouseCursor != nullptr)
				this->kbm->mouseCursor->updateAnimation(deltaTime);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// DirectInput / Joystick ////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	// initialize Direct Input
	util::Expected<void> InputHandler::initializeDirectInput(const HINSTANCE& hInstance, const HWND& appWindow)
	{
		// initialize the main DirectInput 8 device
		if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&dev, NULL)))
			return std::runtime_error("Critical error: Unable to create the main DirectInput 8 COM object!");

		// enumerate all available game controllers
		if (FAILED(dev->EnumDevices(DI8DEVCLASS_GAMECTRL, &staticEnumerateGameControllers, this, DIEDFL_ATTACHEDONLY)))
			return std::runtime_error("Critical error: Unable to enumerate input devices!");

		// if no controllers are available, there is nothing left to do, the game will have to run with keyboard and mouse only
		if (gameControllersDI.empty())
		{
			if (activeJoystick && !activeGamepad)
			{
				// load keyboard controls
				activeJoystick = false;
				loadGameCommands();
			}
		}

		if (!activeJoystick && !activeGamepad)
			// no joystick or gamepad support desired -> break
			return {};

		// if there are game controllers, we will have to set them up
		// this will have to be redone each time the user selects a new controller
		// for now we select the first controller as the active controller
		currentlyActiveGameController = 0;
		util::Expected<void> result = initializeGameController(appWindow);
		if(!result.isValid())
			return std::runtime_error("Critical error: Unable to initialize game controller!");

		// return success
		return { };
	}

	// enumerate available game controllers
	BOOL CALLBACK InputHandler::staticEnumerateGameControllers(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef)
	{
		InputHandler* inputHandlerInstance = (InputHandler*)pvRef;
		return inputHandlerInstance->enumerateGameControllers(devInst);
	}
	BOOL InputHandler::enumerateGameControllers(LPCDIDEVICEINSTANCE devInst)
	{
		// enumerate devices
		LPDIRECTINPUTDEVICE8 gameController;
		
		// create interface for the current game controller
		if (FAILED(dev->CreateDevice(devInst->guidInstance, &gameController, NULL)))
			return DIENUM_CONTINUE;
		else
		{
			// store the game controller
			gameControllersDI.push_back(gameController); // (std::make_pair<std::wstring, LPDIRECTINPUTDEVICE8>(devInst->tszProductName, std::move(gameController)));
			return DIENUM_CONTINUE;
		}
	}

	// initialize game controller
	util::Expected<void> InputHandler::initializeGameController(const HWND& appWindow)
	{
		if (currentlyActiveGameController < 0 || currentlyActiveGameController >= gameControllersDI.size())
			return std::runtime_error("Critical error: Game controller index out of range!");

		// get currently active game controller
		LPDIRECTINPUTDEVICE8 gameController = gameControllersDI[currentlyActiveGameController];

		// set cooperative level
		//  - DISCL_BACKGROUND: receive notifications when the application is in the background as well as in the foreground
		//  - DISCL_EXCLUSIVE: exclusive access: no other application can request exclusive access to the game controller
		if (FAILED(gameController->SetCooperativeLevel(appWindow, DISCL_BACKGROUND | DISCL_EXCLUSIVE)))
			return std::runtime_error("Critical error: Unable to set the cooperative level for the game controller!");

		// set data format
		if (FAILED(gameController->SetDataFormat(&c_dfDIJoystick)))
			return std::runtime_error("Critical error: Unable to set data format for the game controller!");

		// set range and dead zone of joystick axes
		if (FAILED(gameController->EnumObjects(&staticSetGameControllerAxesRanges, gameController, DIDFT_AXIS)))
			throw "Critical error: Unable to set axis ranges of game controllers!";

		// acquire the controller
		if (FAILED(gameController->Acquire()))
			throw "Critical error: Unable to acquire the game controller!";

		// create the joystick class object
		if (joystick != nullptr)
			delete joystick;
		joystick = new Joystick(gameController);

		// return success
		return { };
	}

	// set controller axes
	BOOL CALLBACK InputHandler::staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef)
	{
		// the game controller
		LPDIRECTINPUTDEVICE8 gameController = (LPDIRECTINPUTDEVICE8)pvRef;
		gameController->Unacquire();
		
		// structure to hold game controller range properties
		DIPROPRANGE gameControllerRange;

		// set the range to -100 and 100
		gameControllerRange.lMin = -100;
		gameControllerRange.lMax = 100;

		// set the size of the structure
		gameControllerRange.diph.dwSize = sizeof(DIPROPRANGE);
		gameControllerRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

		// set the object that we want to change		
		gameControllerRange.diph.dwHow = DIPH_BYID;
		gameControllerRange.diph.dwObj = devObjInst->dwType;

		// now set the range for the axis		
		if (FAILED(gameController->SetProperty(DIPROP_RANGE, &gameControllerRange.diph))) {
			return DIENUM_STOP;
		}

		// structure to hold game controller axis dead zone
		DIPROPDWORD gameControllerDeadZone;

		// set the dead zone to 1%
		gameControllerDeadZone.dwData = 100;

		// set the size of the structure
		gameControllerDeadZone.diph.dwSize = sizeof(DIPROPDWORD);
		gameControllerDeadZone.diph.dwHeaderSize = sizeof(DIPROPHEADER);

		// set the object that we want to change
		gameControllerDeadZone.diph.dwHow = DIPH_BYID;
		gameControllerDeadZone.diph.dwObj = devObjInst->dwType;

		// now set the dead zone for the axis
		if (FAILED(gameController->SetProperty(DIPROP_DEADZONE, &gameControllerDeadZone.diph)))
			return DIENUM_STOP;

		return DIENUM_CONTINUE;
	}

	// poll
	HRESULT Joystick::poll()
	{
		HRESULT hr;

		// store the current state
		CopyMemory(&previousState, &currentState, sizeof(DIJOYSTATE));
		ZeroMemory(&currentState, sizeof(DIJOYSTATE));

		// poll the device to read the current state
		hr = dev->Poll();

		if (FAILED(hr))
		{
			// DirectInput lost the device, try to re-acquire it
			hr = dev->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = dev->Acquire();

			// return if a fatal error is encountered
			if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
				return E_FAIL;

			// if another application has control of this device, we have to wait for our turn
			if (hr == DIERR_OTHERAPPHASPRIO)
				return S_OK;
		}

		// now if everything is okay, we can get the state of the device
		if (FAILED(hr = dev->GetDeviceState(sizeof(DIJOYSTATE), &currentState)))
			return hr;

		return S_OK;
	}

	// button states
	const bool Joystick::isPressed(const JoystickButtons button) const
	{
		if (button < JoystickButtons::EndPOV)
		{
			// POV button
			switch (button)
			{
				//   0 degrees: north
				//  90 degrees: east
				// 180 degrees: south
				// 270 degrees: west
			case JoystickButtons::JoyPOV_Up:
				return currentState.rgdwPOV[0] == 0;
			case JoystickButtons::JoyPOV_Right:
				return currentState.rgdwPOV[0] == 9000;
			case JoystickButtons::JoyPOV_Down:
				return currentState.rgdwPOV[0] == 18000;
			case JoystickButtons::JoyPOV_Left:
				return currentState.rgdwPOV[0] == 27000;
			}
		}

		if (button > JoystickButtons::EndPOV && button < JoystickButtons::EndButtons)
		{	
			// regular button
			JoystickButtons btn = (JoystickButtons)(button - 5);
			return (currentState.rgbButtons[btn] & 0x80) ? 1 : 0;
		}

		return false;
	}
	const bool Joystick::wasPressed(const JoystickButtons button) const
	{
		if (button < JoystickButtons::EndPOV)
		{
			// POV button
			switch (button)
			{
				//   0 degrees: north
				//  90 degrees: east
				// 180 degrees: south
				// 270 degrees: west
			case JoystickButtons::JoyPOV_Up:
				return previousState.rgdwPOV[0] == 0;
			case JoystickButtons::JoyPOV_Right:
				return previousState.rgdwPOV[0] == 9000;
			case JoystickButtons::JoyPOV_Down:
				return previousState.rgdwPOV[0] == 18000;
			case JoystickButtons::JoyPOV_Left:
				return previousState.rgdwPOV[0] == 27000;
			}
		}

		if (button > JoystickButtons::EndPOV && button < JoystickButtons::EndButtons)
		{
			// regular button
			JoystickButtons btn = (JoystickButtons)(button - 5);
			return (previousState.rgbButtons[btn] & 0x80) ? 1 : 0;
		}
		
		return false;
	}
	const KeyState Joystick::getButtonState(const JoystickButtons button) const
	{
		if (button == JoystickButtons::EndButtons || button == JoystickButtons::EndPOV)
			return KeyState::StillReleased;

		if (wasPressed(button) == 1)
			if (isPressed(button) == 1)
				return KeyState::StillPressed;
			else
				return KeyState::JustReleased;
		else
			if (isPressed(button) == 1)
				return KeyState::JustPressed;
			else
				return KeyState::StillReleased;
	}


	

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// XInput / Gamepad //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	
	// XInput with DirectInput
	const BOOL InputHandler::isXInputDevice(const GUID* const pGuidProductFromDirectInput) const
	{
		// check a game controller device ID to see if it contains "IG_" (ex. "VID_045E&PID_028E&IG_00")
		// if it does, then it's an XInput device
		
		// unfortunately this information can not be found by just using DirectInput 
		IWbemLocator*           pIWbemLocator = NULL;
		IEnumWbemClassObject*   pEnumDevices = NULL;
		IWbemClassObject*       pDevices[20] = { 0 };
		IWbemServices*          pIWbemServices = NULL;
		BSTR                    bstrNamespace = NULL;
		BSTR                    bstrDeviceID = NULL;
		BSTR                    bstrClassName = NULL;
		DWORD                   uReturned = 0;
		bool                    bIsXinputDevice = false;
		UINT                    iDevice = 0;
		VARIANT                 var;
		HRESULT                 hr;

		// coinit
		hr = CoInitialize(NULL);
		bool bCleanupCOM = SUCCEEDED(hr);

		// create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWbemLocator),
			(LPVOID*)&pIWbemLocator);
		if (FAILED(hr) || pIWbemLocator == NULL)
			goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == NULL) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (bstrClassName == NULL) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");          if (bstrDeviceID == NULL)  goto LCleanup;

		// connect to WMI 
		hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L,
			0L, NULL, NULL, &pIWbemServices);
		if (FAILED(hr) || pIWbemServices == NULL)
			goto LCleanup;

		// switch security level to IMPERSONATE
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == NULL)
			goto LCleanup;

		// loop over all devices
		for (;;)
		{
			// get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr))
				goto LCleanup;
			if (uReturned == 0)
				break;

			for (iDevice = 0; iDevice<uReturned; iDevice++)
			{
				// for each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
				{
					// check if the device ID contains "IG_"
					// if it does, then it's an XInput device
					// this information can not be found from DirectInput 
					if (wcsstr(var.bstrVal, L"IG_"))
					{
						// if it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf_s(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;
						WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf_s(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						// compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == pGuidProductFromDirectInput->Data1)
						{
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		if (bstrNamespace)
			SysFreeString(bstrNamespace);
		if (bstrDeviceID)
			SysFreeString(bstrDeviceID);
		if (bstrClassName)
			SysFreeString(bstrClassName);

		for (iDevice = 0; iDevice<20; iDevice++)
			SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);

		if (bCleanupCOM)
			CoUninitialize();

		return bIsXinputDevice;
	}
	
	// initialize XInput gamepads
	// currently only one player is supported
	const bool InputHandler::initializeXInputGamepads()
	{
		// we currently only support 1 player
		if (nPlayers != 1)
			return false;

		// try to get a gamepad for each player
		for (unsigned int p = 0; p < nPlayers; p++)
		{
			// get port
			int playerID = -1;
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));

			for (DWORD i = 0; i < XUSER_MAX_COUNT && playerID == -1; i++)
			{
				ZeroMemory(&state, sizeof(XINPUT_STATE));

				if (XInputGetState(i, &state) == ERROR_SUCCESS)
					playerID = i;
			}

			if (playerID != -1)
			{
				// create gamepad
				gameControllersXI.push_back(new Gamepad(playerID));
			}
		}

		nGamepads = (unsigned int)gameControllersXI.size();
		if (nGamepads != nPlayers)
			return false;

		// select current player
		currentlyActivePlayer = 0;
		gamepad = gameControllersXI[0];
		
		return true;
	}

	// button functions
	const bool Gamepad::isPressed(const WORD button) const
	{
		return (currentState.Gamepad.wButtons & button) != 0;
	}
	const bool Gamepad::wasPressed(const WORD button) const
	{
		return (previousState.Gamepad.wButtons & button) != 0;
	}
	const KeyState Gamepad::getButtonState(const WORD button) const
	{
		if (wasPressed(button))
			if (isPressed(button))
				return KeyState::StillPressed;
			else
				return KeyState::JustReleased;
		else
			if (isPressed(button))
				return KeyState::JustPressed;
			else
				return KeyState::StillReleased;
	}
	
	// trigger functions
	const KeyState Gamepad::getDigitalTriggerState(const unsigned int i) const
	{
		if (i == 0)
		{
			// left trigger button
			if (previousState.Gamepad.bLeftTrigger == 255)
				if (currentState.Gamepad.bLeftTrigger == 255)
					return KeyState::StillPressed;
				else
					return KeyState::JustReleased;
			else
				if (currentState.Gamepad.bLeftTrigger == 255)
					return KeyState::JustPressed;
				else
					return KeyState::StillReleased;
		}
		else
		{
			// right trigger button
			if (previousState.Gamepad.bRightTrigger == 255)
				if (currentState.Gamepad.bRightTrigger == 255)
					return KeyState::StillPressed;
				else
					return KeyState::JustReleased;
			else
				if (currentState.Gamepad.bRightTrigger == 255)
					return KeyState::JustPressed;
				else
					return KeyState::StillReleased;
		}
	}
	const float Gamepad::getAnalogZL() const
	{
		return (float)currentState.Gamepad.bLeftTrigger / 255.0f;
	}
	const float Gamepad::getAnalogZR() const
	{
		return (float)currentState.Gamepad.bRightTrigger / 255.0f;
	}
	const float Gamepad::getRelativeZL() const
	{
		return ((float)(currentState.Gamepad.bLeftTrigger - previousState.Gamepad.bLeftTrigger) / 255.0f);
	}
	const float Gamepad::getRelativeZR() const
	{
		return ((float)(currentState.Gamepad.bRightTrigger - previousState.Gamepad.bRightTrigger) / 255.0f);
	}

	// thumb sticks
	const float InputHandler::getLX() const
	{
		if (activeGamepad)
			return gamepad->thumbStickLeft->x;
		else
			return 0.0f;
	}
	const float InputHandler::getLY() const
	{
		if (activeGamepad)
			return gamepad->thumbStickLeft->y;
		else
			return 0.0f;
	}
	const float InputHandler::getRX() const
	{
		if (activeGamepad)
			return gamepad->thumbStickRight->x;
		else
			return 0.0f;
	}
	const float InputHandler::getRY() const
	{
		if (activeGamepad)
			return gamepad->thumbStickRight->y;
		return 0.0f;
	}
	
	// battery level
	const BYTE InputHandler::getBatteryLevel() const
	{
		// get battery level
		return gamepad->battery.BatteryLevel;
	}
	
	// vibration
	void InputHandler::vibrateGamepad(const unsigned int leftSpeed, const unsigned int rightSpeed) const
	{
		if(activeGamepad)
			gamepad->vibrate(leftSpeed, rightSpeed);
	}
	void InputHandler::vibrateGamepad(const float normalizedLeftSpeed, const float normalizedRightSpeed) const
	{
		if(activeGamepad)
			gamepad->vibrate(normalizedLeftSpeed, normalizedRightSpeed);
	}
	void Gamepad::vibrate(unsigned int leftSpeed, unsigned int rightSpeed)
	{
		if (leftSpeed < 0)
			leftSpeed = 0;
		if (rightSpeed < 0)
			rightSpeed = 0;

		if (leftSpeed > maxMotorSpeed)
			leftSpeed = maxMotorSpeed;
		if (rightSpeed > maxMotorSpeed)
			rightSpeed = maxMotorSpeed;

		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = (WORD)leftSpeed;
		vibration.wRightMotorSpeed = (WORD)rightSpeed;

		XInputSetState(playerID, &vibration);
	}
	void Gamepad::vibrate(float normalizedLeftSpeed, float normalizedRightSpeed)
	{
		if (normalizedLeftSpeed > 1.0f)
			normalizedLeftSpeed = 1.0f;
		else if (normalizedLeftSpeed < 0.0f)
			normalizedLeftSpeed = 0.0f;

		if (normalizedRightSpeed > 1.0f)
			normalizedRightSpeed = 1.0f;
		else if (normalizedRightSpeed < 0.0f)
			normalizedRightSpeed = 0.0f;

		vibrate((unsigned int)(normalizedLeftSpeed*maxMotorSpeed), (unsigned int)(normalizedRightSpeed*maxMotorSpeed));
	}

	// acquire input
	util::Expected<void> Gamepad::poll()
	{
		// check connection
		if (!checkConnection())
			return std::runtime_error("Critical error: Connection to the gamepad was lost!");

		// get battery state
		XInputGetBatteryInformation(playerID, BATTERY_DEVTYPE_GAMEPAD, &battery);

		// save state
		CopyMemory(&previousState, &currentState, sizeof(XINPUT_STATE));
		ZeroMemory(&currentState, sizeof(XINPUT_STATE));

		// get current state
		if (XInputGetState(playerID, &currentState) != ERROR_SUCCESS)
			return std::runtime_error("Critical error: Unable to poll gamepad!");

		if (previousState.dwPacketNumber == currentState.dwPacketNumber)
			// there was no change
			return { };

		// get axes
		thumbStickLeft->x = (float)currentState.Gamepad.sThumbLX;
		thumbStickLeft->y = (float)currentState.Gamepad.sThumbLY;
		thumbStickRight->x = (float)currentState.Gamepad.sThumbRX;
		thumbStickRight->y = (float)currentState.Gamepad.sThumbRY;

		// get relative axis between -1.0f and 1.0f while considering a dead zone of 10%

		// left thumb stick

		// if within deadzone, set to 0
		if (thumbStickLeft->getSquareLength() < deadzone*deadzone)
			thumbStickLeft->x = thumbStickLeft->y = 0;
		else
		{
			// calculate the percentage between the deadzone and the maximal values
			float percentage = (thumbStickLeft->getLength() - deadzone) / (maxValue - deadzone);

			// normalize vector and multiply to get the correct final value
			thumbStickLeft->normalize(thumbStickLeft->length);
			*thumbStickLeft *= percentage;

			if (thumbStickLeft->x > 1.0f)
				thumbStickLeft->x = 1.0f;
			if (thumbStickLeft->x < -1.0f)
				thumbStickLeft->x = -1.0f;
			
			if (thumbStickLeft->y > 1.0f)
				thumbStickLeft->y = 1.0f;
			if (thumbStickLeft->y < -1.0f)
				thumbStickLeft->y = -1.0f;
		}

		// right thumb stick

		// if within deadzone, set to 0
		if (thumbStickRight->getSquareLength() < deadzone * deadzone)
			thumbStickRight->x = thumbStickRight->y = 0;
		else
		{
			// calculate the percentage between the deadzone and the maximal values
			float percentage = (thumbStickRight->getLength() - deadzone) / (maxValue - deadzone);

			// normalize vector and multiply to get the correct final value
			thumbStickRight->normalize(thumbStickRight->length);
			*thumbStickRight *= percentage;

			if (thumbStickRight->x > 1.0f)
				thumbStickRight->x = 1.0f;
			if (thumbStickRight->x < -1.0f)
				thumbStickRight->x = -1.0f;

			if (thumbStickRight->y > 1.0f)
				thumbStickRight->y = 1.0f;
			if (thumbStickRight->y < -1.0f)
				thumbStickRight->y = -1.0f;
		}

		// return success
		return { };
	}
	bool Gamepad::checkConnection()
	{
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));

		if (XInputGetState(playerID, &state) == ERROR_SUCCESS)
			return true;

		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Bindings /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> InputHandler::saveGameCommands() const
	{
		std::ofstream keyBindingsOut;

		if (activeGamepad)
			// gamepad input
			keyBindingsOut = std::ofstream(this->keyBindingsFileGamepad, std::ios::out);
		else if(activeJoystick)
			// joystick input
			keyBindingsOut = std::ofstream(this->keyBindingsFileJoystick, std::ios::out);
		else if(activeKeyboard || activeMouse)
			// keyboard input
			keyBindingsOut = std::ofstream(this->keyBindingsFileKeyboard, std::ios::out);
		else
			return std::runtime_error("Critical error: Unable to deduce input device!");

		// open text archive
		boost::archive::text_oarchive oa(keyBindingsOut);
		
		const std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;

		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;

		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to deduce input device!");

		for (auto gameCommand : *keyMap)
		{
			oa << gameCommand.first;
			oa << gameCommand.second;
		}

		keyBindingsOut.close();

		// return success
		return { };
	}
	util::Expected<void> InputHandler::loadGameCommands()
	{
		// create default key bindings if file does not yet exist
		if (activeGamepad)
		{
			if (GetFileAttributes(keyBindingsFileGamepad.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				setDefaultKeyMap();
				saveGameCommands();
			}
		}
		else if (activeJoystick)
		{
			if (GetFileAttributes(keyBindingsFileJoystick.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				setDefaultKeyMap();
				saveGameCommands();
			}
		}
		else if (activeKeyboard || activeMouse)
		{
			if (GetFileAttributes(keyBindingsFileKeyboard.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				setDefaultKeyMap();
				saveGameCommands();
			}
		}

		std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;
		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;

		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to find input device!");

		// clear map
		keyMap->clear();

		// load data
		GameCommands gcs;

		std::ifstream keyBindingsIn;

		if (!activeJoystick && !activeGamepad)
			// keyboard input
			keyBindingsIn = std::ifstream(this->keyBindingsFileKeyboard, std::ios::in);
		else if (activeJoystick)
			// joystick input
			keyBindingsIn = std::ifstream(this->keyBindingsFileJoystick, std::ios::in);
		else if (activeGamepad)
			// gamepad input
			keyBindingsIn = std::ifstream(this->keyBindingsFileGamepad, std::ios::in);
		else
			return std::runtime_error("Critical error: Unable to deduce input device!");
		
		// open the file
		boost::archive::text_iarchive ia(keyBindingsIn);
		
		// populate map with information from file
		while (!keyBindingsIn.eof())
		{
			ia >> gcs;
			ia >> unmarshalledGameCommand;
			keyMap->insert(std::pair<GameCommands, GameCommand*>(gcs, unmarshalledGameCommand));
		}

		// close the file
		keyBindingsIn.close();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Listening ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
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

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Getters //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> InputHandler::getKeysMappedToCommand(const GameCommands gameCommand, std::vector<std::vector<BindInfo> >& map) const
	{
		const std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;

		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;

		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to find input device!");

		std::vector<std::vector<BindInfo> > vecBI;
		for(auto x : *keyMap)
			if (x.first == gameCommand)
				map.push_back(x.second->chord);

		// return success
		return { };
	}
	util::Expected<void> InputHandler::getCommandsMappedToGameAction(const GameCommands gameCommand, std::vector<GameCommand*>& commands) const
	{
		const std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;

		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;

		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to find input device!");

		for (auto x : *keyMap)
			if (x.first == gameCommand)
				commands.push_back(x.second);

		// return success
		return { };
	}
	util::Expected<void> InputHandler::insertNewCommand(GameCommands gameCommand, GameCommand& command)
	{
		std::unordered_multimap<GameCommands, GameCommand*>* keyMap = nullptr;

		// get keyMap
		if (activeGamepad)
			// gamepad input
			keyMap = &keyMapGamepad;
		else if (activeJoystick)
			// joystick input
			keyMap = &keyMapJoystick;
		else
			// keyboard input
			keyMap = &keyMapKeyboard;

		if (keyMap == nullptr)
			return std::runtime_error("Critical error: Unable to find input device!");

		keyMap->insert(std::pair<GameCommands, GameCommand*>(gameCommand, &command));

		// return succcess
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Configuration File ///////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void InputHandler::readConfigFile()
	{
		if (dxApp.getFileSystemComponent().hasValidConfigurationFile())
		{
			// configuration file exists, try to read from it
			std::wstring pathToPrefFile = dxApp.getFileSystemComponent().getPathToConfigurationFiles() + L"\\" + dxApp.getFileSystemComponent().getPrefsFile();

			try
			{
				sol::state lua;
				lua.script_file(util::StringConverter::ws2s(pathToPrefFile));

				// read from the configuration file, default to false
				activeJoystick = lua["config"]["joystick"].get_or(false);
				activeGamepad = lua["config"]["gamepad"].get_or(false);
#ifndef NDEBUG
				std::stringstream res;
				res << "The game controller states were read from the Lua configuration file: joystick: " << std::boolalpha << activeJoystick << " --- gamepad: " << std::boolalpha << activeGamepad << ".";
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>(res.str());
#endif
			}
			catch (std::exception)
			{
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Unable to read configuration file. Joystick and gamepad support disabled!");
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Messages /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> InputHandler::onMessage(const core::Depesche& depesche)
	{
		if (depesche.type == core::DepescheTypes::Gamepad)
		{
			float vibrationSpeed = *(float *)depesche.message;
			gamepad->vibrate(vibrationSpeed, vibrationSpeed);
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Key Codes ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	const util::Expected<std::wstring> InputHandler::getKeyName(const unsigned int keyCode) const
	{
		if (keyCode >= 0 && keyCode < joystickBegin)
		{
			// this is a keyboard or mouse code

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
					return (std::wstring)L"Unknown Key";
				}
			}
		}
		
		if (keyCode >= joystickBegin && keyCode < gamepadBegin)
		{
			// joystick button
			unsigned int button = keyCode % joystickBegin;
			return (std::wstring)enumToString(JoystickButtons(button));
		}

		if (keyCode > gamepadBegin)
		{
			// gamepad button
			WORD button = (WORD)(keyCode - gamepadBegin);

			// left and right trigger as digital buttons
			if (keyCode == ZL)
				button = (WORD)ZL;
			if (keyCode == ZR)
				button = (WORD)ZR;

			std::wstring buttonName;

			switch (button)
			{
			case XINPUT_GAMEPAD_A:
				buttonName = L"A";
				break;

			case XINPUT_GAMEPAD_B:
				buttonName = L"B";
				break;

			case XINPUT_GAMEPAD_X:
				buttonName = L"X";
				break;

			case XINPUT_GAMEPAD_Y:
				buttonName = L"Y";
				break;

			case XINPUT_GAMEPAD_LEFT_THUMB:
				buttonName = L"LT";
				break;

			case XINPUT_GAMEPAD_RIGHT_THUMB:
				buttonName = L"RT";
				break;

			case XINPUT_GAMEPAD_LEFT_SHOULDER:
				buttonName = L"LS";
				break;

			case XINPUT_GAMEPAD_RIGHT_SHOULDER:
				buttonName = L"RS";
				break;

			case XINPUT_GAMEPAD_DPAD_LEFT:
				buttonName = L"Left";
				break;

			case XINPUT_GAMEPAD_DPAD_RIGHT:
				buttonName = L"Right";
				break;

			case XINPUT_GAMEPAD_DPAD_UP:
				buttonName = L"Up";
				break;

			case XINPUT_GAMEPAD_DPAD_DOWN:
				buttonName = L"Down";
				break;

			case XINPUT_GAMEPAD_START:
				buttonName = L"Start";
				break;

			case XINPUT_GAMEPAD_BACK:
				buttonName = L"Select";
				break;

			case 18000:
				buttonName = L"ZL";
				break;

			case 18001:
				buttonName = L"ZR";
				break;

			default: return (std::wstring)L"Unknown Key";
			}

			return buttonName;
		}

		return (std::wstring)L"Unknwon Key";
		
	}
}