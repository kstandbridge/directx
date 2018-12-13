// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ stringstream
#include <sstream>

// bell0bytes core
#include "app.h"
#include "playState.h"

// bell0bytes UI
#include "HeadsUpDisplayState.h"
//#include "mainMenuState.h"
#include "gameMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputComponent.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "sprites.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponentWrite.h"

// bell0bytes file system
#include "fileSystemComponent.h"


// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	HeadsUpDisplayState::HeadsUpDisplayState(core::DirectXApp& dxApp, const std::wstring& name) : GameState(dxApp, name)
	{

	}

	HeadsUpDisplayState::~HeadsUpDisplayState()
	{

	}

	HeadsUpDisplayState& HeadsUpDisplayState::createInstance(core::DirectXApp& dxApp, const std::wstring& stateName)
	{
		static HeadsUpDisplayState instance(dxApp, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// hide the standard cursor
		ShowCursor(false);

		// allow mouse and keyboard input
		dxApp.getInputComponent().getInputHandler().activeMouse = false;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;
		isPaused = false;

		if (firstCreation)
		{
			// create text formats
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 48.0f, DWRITE_TEXT_ALIGNMENT_CENTER, hudFormat);
			if (!result.isValid())
				return result;

			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 32.0f, DWRITE_TEXT_ALIGNMENT_LEADING, batteryLevelFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wostringstream hud;
			hud << L"Cats still alive: " << nActiveCats;
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(hud, hudFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, hudLayout);
			if (!result.wasSuccessful())
				return result;

			std::wstring batteryLevelText = L"Gamepad Battery Level";
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(batteryLevelText, batteryLevelFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, batteryLevelLayout);
			if (!result.wasSuccessful())
				return result;
		}

		// initialize gamepad battery level button
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		// cycle
		cycle.name = L"Battery Empty";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 24;
		cycle.height = 15;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Battery Low";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 24;
		cycle.height = 15;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Battery Medium";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 24;
		cycle.height = 15;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Battery Full";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 24;
		cycle.height = 15;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Icons, L"iconBatteryLevels.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		// create sprite

		// add button to the list
		try { iconBatteryLevel = new graphics::AnimatedSprite(d2d, animations, 0); }
		catch (std::runtime_error& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		firstCreation = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::pause()
	{
		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> HeadsUpDisplayState::resume()
	{
		// allow mouse and keyboard input
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;

		isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::onMessage(const core::Depesche& depesche)
	{
		if (depesche.type == core::DepescheTypes::ActiveKeyMap)
		{
			input::InputHandler* ih = (input::InputHandler*)depesche.sender;

			if (!isPaused)
				if (!ih->isListening())
					// handle interface messages
					return handleInput(ih->activeKeyMap);
		}
		else if(depesche.type == core::DepescheTypes::Damage)
			nActiveCats = *(unsigned int*)depesche.message;

		// return success
		return { };
	}

	util::Expected<void> HeadsUpDisplayState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::GameCommands::Back:
				if (!dxApp.pushGameState(&UI::GameMenuState::createInstance(dxApp, L"Game Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to push game menu state!");
				break;

			case input::GameCommands::ShowFPS:
				dxApp.toggleFPS();
				break;
			}
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::update(const double /*deltaTime*/)
	{
		if (dxApp.getInputComponent().getInputHandler().activeGamepad)
		{
			// get battery level of gamepad
			unsigned int batteryLevel = dxApp.getInputComponent().getInputHandler().getBatteryLevel();
			iconBatteryLevel->changeAnimation(batteryLevel);
		}

		// update cats alive text
		util::Expected<void> result;
		hudLayout.ReleaseAndGetAddressOf();
		std::wostringstream hud;
		if (nActiveCats > 0)
			hud << L"Katzen op dem Cosmo senger Wiss: " << nActiveCats;
		else
			hud << L"De Cosmo huet d'Katzen verdriwwen!";

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(hud, hudFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, hudLayout);
		if (!result.wasSuccessful())
			return result;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::render(const double /*farSeer*/)
	{
		if (!isPaused)
		{
			if(nActiveCats > 0)
				dxApp.getGraphicsComponent().getWriteComponent().printText(0, 900, hudLayout.Get());
			else
				dxApp.getGraphicsComponent().getWriteComponent().printText(0, 540, hudLayout.Get());

			dxApp.getGraphicsComponent().getWriteComponent().printText(10, 900, batteryLevelLayout.Get(), 0.25f);

			// battery level icon
			//iconBatteryLevel->draw(1, 50, 1000);

			// print FPS information
			dxApp.getGraphicsComponent().getWriteComponent().printFPS();
		}
		else
		{
			dxApp.getGraphicsComponent().getWriteComponent().printText(0, 800, hudLayout.Get(), 0.25f);
			dxApp.getGraphicsComponent().getWriteComponent().printText(10, 900, batteryLevelLayout.Get());

			// battery level icon
			iconBatteryLevel->draw(1, 50, 1000);
		}

		// print FPS information
		dxApp.getGraphicsComponent().getWriteComponent().printFPS();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> HeadsUpDisplayState::shutdown()
	{
		// delete battery level icon
		delete iconBatteryLevel;

		isPaused = true;

		// return success
		return {};
	}
}