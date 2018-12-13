#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		31/05/2018 - Lenningen - Luxembourg
*
* Desc:		Event based input handler
*
* History:	- 05/06/2018: added keyboard support
*			- 11/06/2018: added mouse support
*			- 12/06/2018: boost serialization to save and load key bindings
*			- 13/06/2018: the input handler now sends notifications to the game states
*			- 23/06/2018: added DirectInput support
*			- 24/06/2018: added XInput support
*
* ToDo:		- memory leak in load function (possible bug in boost serialization? singleton never gets deleted?)
*			- exception when joystick is set to true but no joystick was found
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and COM
#include <Windows.h>
#include <wrl.h>

// c++ containers
#include <unordered_map>
#include <array>

// DirectInput
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

// XInput
#include <Xinput.h>
#pragma comment (lib, "xinput.lib")

// bell0bytes util
#include "observer.h"			// observer pattern

// bell0bytes core
#include "depesche.h"			// event queue

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace boost
{
	namespace serialization
	{
		class access;
	}
}

namespace util
{
	template<typename T>
	class Expected;
}

namespace graphics
{
	class AnimatedSprite;
}

namespace core
{
	class DirectXApp;
}

namespace maths
{
	struct Vector2F;
}

namespace input
{
	// enumerate all game commands
	enum Events : int;																// enumeration of all standard application events
	enum GameCommands : int;														// enumeration of all possible game commands
	enum JoystickButtons : int;														// enumeration of all joystick buttons
	enum KeyState { StillReleased, JustPressed, StillPressed, JustReleased };		// enumeration of all possible key states
	
	// structure to combine key codes and key states
	// each game command can have several bindings (chord), i.e. toggle show FPS = 'shift'+'control'+'FPS'
	struct BindInfo
	{
	private:
		unsigned int keyCode;		// the actual key code
		KeyState keyState;			// the state the above specified key has to be in for the "binding" to become active
	
		template <typename Archive>				// define serialization, both save and load
		void serialize(Archive& ar, const unsigned int /*version*/)
		{
			ar & keyCode & keyState;
		}

	public:
		// constructors and destructor
		BindInfo();
		BindInfo(const unsigned int keyCode, const KeyState keyState);
		~BindInfo() {};

		// getters
		const unsigned int getKeyCode() const { return keyCode; };

		friend struct GameCommand;
		friend class InputHandler;
		friend class boost::serialization::access;
	};

	// structure to map a single game command to a chord of key strokes (see above)
	struct GameCommand
	{
	private:
		std::wstring name;						// human readable name
		std::vector<BindInfo> chord;			// the chord mapped to this command, i.e. "shift"+"control"+"F"

		// serialization
		template <typename Archive>				// define serialization, both save and load
		void serialize(Archive& ar, const unsigned int /*version*/)
		{ ar & name & chord; }

		// set new chord
		void setChord(const std::vector<BindInfo>* const bi) { chord = *bi; };

	public:
		// constructors and destructor
		GameCommand();
		GameCommand(const std::wstring& name);
		GameCommand(const std::wstring& name, const unsigned int keyCode, const KeyState keyState);
		GameCommand(const std::wstring& name, const BindInfo& bi);
		GameCommand(const std::wstring& name, const std::vector<BindInfo>& chord);
		~GameCommand() {};

		// get chord
		const std::vector<BindInfo> getChord() const { return chord; };
		void setChord(std::vector<BindInfo> newChord) { chord = newChord; };

		friend class InputHandler;
		friend class boost::serialization::access;
	};

	// the keyboard and mouse class
	class KeyboardAndMouse
	{
	private:
		std::array<bool, 256> currentState;					// the state of the keyboard keys and mouse buttons in the current frame
		std::array<bool, 256> previousState;				// the state of the keyboard keys and mouse buttons in the previous frame

		graphics::AnimatedSprite* mouseCursor;				// the sprite of the mouse cursor
		long mouseX, mouseY;								// the position of the mouse cursor

	public:
		KeyboardAndMouse();
		KeyboardAndMouse(graphics::AnimatedSprite* const mouseCursor);
		~KeyboardAndMouse();
		
		friend class InputHandler;
	};

	// the joystick class (DirectInput)
	class Joystick
	{
	private:
		LPDIRECTINPUTDEVICE8 const dev;					// the actual joystick device
		std::wstring name;								// the name of the game controller

		DIJOYSTATE currentState;						// the state of the joystick in the current frame
		DIJOYSTATE previousState;						// the state of the joystick in the previous frame

		unsigned int numberOfAxes;						// the number of axes on the joystick
		unsigned int numberOfPOVs;						// the number of POVs on the joystick
		unsigned int numberOfButtons;					// the number of buttons on the joystick

		HRESULT poll();									// polls the state of the joystick
		const bool isPressed(const JoystickButtons button) const;	// returns true iff the button is pressed in the current frame
		const bool wasPressed(const JoystickButtons button) const;	// returns true iff the button was pressed in the previous frame
		
		// state of a button, see enum above
		const KeyState getButtonState(const JoystickButtons button) const;

	public:
		Joystick(LPDIRECTINPUTDEVICE8 const dev);
		~Joystick();

		const std::wstring& getName() const { return name; }

		friend class InputHandler;
	};

	// the gamepad class (XInput)
	class Gamepad
	{
	private:
		XINPUT_STATE currentState;		// the state of the gamepad in the current frame
		XINPUT_STATE previousState;		// the state of the game in the previous frame
		const unsigned int playerID;	// player number (0-3)

		// vector and deadzone
		const float maxValue = 32767;		// maximal value for the axes
		const float deadzone = 6552;		// deadzone of 20%
		maths::Vector2F* thumbStickLeft;	// left thumb stick vector
		maths::Vector2F* thumbStickRight;	// right thumb stick vector

		// vibration
		XINPUT_VIBRATION vibration;					// vibration settings
		const unsigned int maxMotorSpeed = 65535;	// max motor speed
		
		// battery
		XINPUT_BATTERY_INFORMATION battery;			// information about the battery of the gamepad

		// acquire the state of the gamepad
		util::Expected<void> poll();				// get gamepad state
		bool checkConnection();						// check whether the gamepad for the specified player is still connected
	
		// vibrate the gamepad
		void vibrate(unsigned int, unsigned int);
		void vibrate(float, float);
	public:
		// constructor and destructor
		Gamepad(const unsigned int);
		~Gamepad();

		// button functions
		const KeyState getButtonState(const WORD button) const;	// returns the state of a button
		const bool isPressed(const WORD button) const;			// returns true iff the button is pressed
		const bool wasPressed(const WORD button) const;			// returns true iff the button was pressed in the previous frame

		// trigger functions
		const KeyState getDigitalTriggerState(const unsigned int i = 0) const; // returns the state of the left (0) or right (1) trigger
		const float getAnalogZL() const;						// returns how far the trigger button is pressed in normalized values
		const float getAnalogZR() const;
		const float getRelativeZL() const;						// returns how far the trigger button is pressed relative to the last frame
		const float getRelativeZR() const;

		friend class InputHandler;
	};

	// the main input handler class
	// sends notifications to the various game states on user input
	class InputHandler : public core::DepescheSender, public core::DepescheDestination
	{
	private:
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// GENERAL //////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		const std::wstring& keyBindingsFileKeyboard;				// files with key bindings
		const std::wstring& keyBindingsFileJoystick;
		const std::wstring& keyBindingsFileGamepad;

		bool listen;												// listen for any user input
		
		// game command used to unmarshalling
		GameCommand* unmarshalledGameCommand;						// used to load game commands from file

		// polling
		util::Expected<void> update();								// update the active key map

		// on message
		util::Expected<void> onMessage(const core::Depesche&) override;

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// CUSTOM KEYCODES //////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////
		const unsigned int joystickBegin = 256;
		const unsigned int gamepadBegin = 293;
		const unsigned int ZL = 18000;
		const unsigned int ZR = 18001;

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// DIRECT INPUT /////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		IDirectInput8* dev;									// the main DirectInput device
		std::vector<LPDIRECTINPUTDEVICE8> gameControllersDI;// a vector of all available DirectInput game controllers
		unsigned int currentlyActiveGameController;			// the index of the currently active joystick
		Joystick* joystick;									// the joystick class

		// initialize DirectInput
		util::Expected<void> initializeDirectInput(const HINSTANCE& hInstance, const HWND& appWindow);

		// enumerate available devices
		static BOOL CALLBACK staticEnumerateGameControllers(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef);
		BOOL enumerateGameControllers(LPCDIDEVICEINSTANCE devInst);

		// initialize game controller
		util::Expected<void> initializeGameController(const HWND& appWindow);

		// enumerat game controller objects
		static BOOL CALLBACK staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef);

		/////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////// XINPUT /////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		std::vector<Gamepad*> gameControllersXI;				// an array of all available XInput controllers 
		Gamepad* gamepad;										// the currently active gamepad
		unsigned int currentlyActivePlayer;						// the index of the currently active player
		unsigned int nGamepads;									// the number of connected gamepads
		unsigned int nPlayers;									// the number of players
		
		const BOOL isXInputDevice(const GUID* const pGuidProductFromDirectInput) const;		// returns true iff the device is an XInput device
		const bool initializeXInputGamepads();

		/////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////// KEYBOARD AND MOUSE ////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		KeyboardAndMouse* kbm;												// pointer to keyboard and mouse class							
		
		void getKeyboardAndMouseState();									// gets the keyboard and mouse button state, uses GetAsyncKeyState to read the state of all 256 keys
		const KeyState getKeyState(const unsigned int keyCode) const;		// gets the state of the specified key, depending on its state in the previous and the current frame
		inline const bool isPressed(int keyCode) const { return (GetAsyncKeyState(keyCode) & 0x8000) ? 1 : 0; };	// returns true iff the key is down
		
	protected:
		core::DirectXApp& dxApp;												// the main DirectX App
		std::unordered_multimap<GameCommands, GameCommand*> keyMapKeyboard;		// list of all possible keyboard game commands mapped to the appropriate command structure
		std::unordered_multimap<GameCommands, GameCommand*> keyMapJoystick;		// list of all possible joystick game commands mapped to the appropriate command structure
		std::unordered_multimap<GameCommands, GameCommand*> keyMapGamepad;		// list of all possible gamepad game commands mapped to the appropriate command structure
		
		// constructor and destructor
		InputHandler(core::DirectXApp& dxApp, const HINSTANCE& hInstance, const HWND& appWindow, const std::wstring& keyBindingsFileKeyboard, const std::wstring& keyBindingsFileJoystick, const std::wstring& keyBindingsFileGamepad);
		
		// initialization
		virtual void setDefaultKeyMap() = 0;						// set up default controls

		// read whether joystick or gamepad input is desired from Lua file
		void readConfigFile();

	public:
		// input devices
		bool activeMouse;						// true iff mouse input is active
		bool activeKeyboard;					// true iff keyboard input is active
		bool activeJoystick;					// true iff the user desires to control the game via a DirectInput joystick
		bool activeGamepad;						// true iff the user desires to control the game via a XInput gamepad
		
		// destructor
		virtual ~InputHandler();
		
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// GENERAL //////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		std::unordered_map<GameCommands, GameCommand&> activeKeyMap;// list of all active key maps; game acts on each command in this list
		std::vector<input::BindInfo> newChordBindInfo;				// used to set new chords

		// toggle listening for specificially requested user input
		void enableListening();
		void disableListening();
		void resetKeyStates();
		const bool isListening() const { return listen; };


		// game commands
		util::Expected<void> getCommandsMappedToGameAction(const GameCommands gameCommand, std::vector<GameCommand*>&) const;		// used to modify game commands in the UI
		util::Expected<void> getKeysMappedToCommand(const GameCommands gameCommand, std::vector<std::vector<BindInfo> >&) const;	// used to show chords mapped to actions in the UI
		const util::Expected<std::wstring> getKeyName(const unsigned int keyCode) const;									// retrieves the human readable name of a virtual key code

		// load and store game commands
		util::Expected<void> insertNewCommand(GameCommands gameCommand, GameCommand& command);
		util::Expected<void> saveGameCommands() const;
		util::Expected<void> loadGameCommands();

		// acquire user input
		util::Expected<void> acquireInput();			// reads the state of the game controllers
		
		/////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////// KEYBOARD AND MOUSE ////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		
		// draw the mouse cursor
		void drawMouseCursor() const;

		// set custom mouse cursor
		void setMouseCursor(graphics::AnimatedSprite* const mouseCursor) { this->kbm->mouseCursor = mouseCursor; };
		
		// update the mouse cursor
		void changeMouseCursorAnimationCycle(const unsigned int cycle) const;
		void updateMouseCursorAnimation(const double deltaTime) const;
		
		// get position of the mouse
		long getMouseX() const { return kbm->mouseX; };
		long getMouseY() const { return kbm->mouseY; };

		/////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////// XInput / Gamepads /////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// thumb sticks
		const float getLX() const;
		const float getLY() const;
		const float getRX() const;
		const float getRY() const;

		// battery level
		const BYTE getBatteryLevel() const;

		// vibration
		void vibrateGamepad(const unsigned int, const unsigned int) const;
		void vibrateGamepad(const float, const float) const;
	};
}