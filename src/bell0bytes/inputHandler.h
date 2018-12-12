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
*
* ToDo:		- memory leak in load function (possible bug in boost serialization? singleton never gets deleted?)
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// Windows and COM
#include <Windows.h>

// c++ containers
#include <unordered_map>
#include <array>

// bell0bytes util
#include "expected.h"		// error handling
#include "observer.h"		// observer pattern


// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace boost
{
	namespace serialization
	{
		class access;
	}
}

namespace graphics
{
	class AnimatedSprite;
}

namespace core
{
	class DirectXApp;
}

namespace input
{
	// enumerate all game commands
	enum Events : int;																// enumeration of all standard application events
	enum GameCommands : int;														// enumeration of all possible game commands
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
		{
			ar & name & chord;
		}

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
	struct KeyboardAndMouse
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

	// the main input handler class
	// sends notifications to the various game states on user input
	class InputHandler : public util::Subject
	{
	private:
		core::DirectXApp* dxApp;									// the main DirectX App

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// GENERAL //////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		const std::wstring& keyBindingsFile;						// file with key bindings
		bool listen;												// listen for any user input
		
		// game command used to unmarshalling
		GameCommand* unmarshalledGameCommand;						// used to load game commands from file

		// polling
		void update();												// update the active key map
				
		/////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////// KEYBOARD AND MOUSE ////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		KeyboardAndMouse* kbm;										// pointer to keyboard and mouse class							
		
		void getKeyboardAndMouseState();									// gets the keyboard and mouse button state, uses GetAsyncKeyState to read the state of all 256 keys
		const KeyState getKeyState(const unsigned int keyCode) const;		// gets the state of the specified key, depending on its state in the previous and the current frame
		inline const bool isPressed(int keyCode) const { return (GetAsyncKeyState(keyCode) & 0x8000) ? 1 : 0; };	// returns true iff the key is down
		
	protected:
		std::unordered_multimap<GameCommands, GameCommand*> keyMap;		// list of all possible game commands mapped to the appropriate command structure
		
		// constructor and destructor
		InputHandler(core::DirectXApp* const dxApp, const std::wstring& keyBindingsFile);
		virtual ~InputHandler();

		// initialization
		virtual void setDefaultKeyMap() = 0;						// set up default controls

	public:
		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////// GENERAL //////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		std::unordered_map<GameCommands, GameCommand&> activeKeyMap;// list of all active key maps; game acts on each command in this list
		std::vector<input::BindInfo> newChordBindInfo;				// used to set new chords

		// toggle listening for specificially requested user input
		void enableListening();
		void disableListening();
		void resetKeyStates();

		// game commands
		void getCommandsMappedToGameAction(const GameCommands gameCommand, std::vector<GameCommand*>&) const;		// used to modify game commands in the UI
		void getKeysMappedToCommand(const GameCommands gameCommand, std::vector<std::vector<BindInfo> >&) const;	// used to show chords mapped to actions in the UI
		const util::Expected<std::wstring> getKeyName(const unsigned int keyCode) const;									// retrieves the human readable name of a virtual key code

		// load and store game commands
		void insertNewCommand(GameCommands gameCommand, GameCommand& command);
		void saveGameCommands() const;
		void loadGameCommands();

		// acquire user input
		void acquireInput();										// reads the state of the game controllers
		
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
	};
}