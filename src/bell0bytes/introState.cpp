// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <sstream>

// bell0bytes core
#include "app.h"				// the main application class

// bell0bytes UI
#include "introState.h"			// the intro state class header
#include "mainMenuState.h"		// the main menu class

// bell0bytes input
#include "gameCommands.h"		// all possible game commands
#include "inputComponent.h"
#include "inputHandler.h"		// the input handler

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponentWrite.h"
#include "sprites.h"			// sprites for the logos

// bell0bytes file system
#include "fileSystemComponent.h"

// bell0bytes audio system
#include "audioComponent.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	IntroState::IntroState(core::DirectXApp& app, const std::wstring& name) : GameState(app, name), frameTime(0.0f), secondsPerLogo(1.0f), showContinueText(true), showTradeMarkLogos(false)
	{ }

	IntroState::~IntroState()
	{ }

	IntroState& IntroState::createInstance(core::DirectXApp& app, const std::wstring& stateName)
	{
		static IntroState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// hide the standard cursor
		ShowCursor(false);

		// allow keyboard input and disable mouse input
		dxApp.getInputComponent().getInputHandler().activeMouse = false;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;
		
		// create text formats
		if (!createTextFormats().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text formats!");
		
		// create text layouts
		if (!createTextLayouts().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text layouts!");

		// create sprites
		if (!initializeLogoSprites().wasSuccessful())
			return std::runtime_error("Critical error: Unable to initialize logo sprites!");
	
		// load the bell0bytes barking sound
		introMusic = new audio::SoundEvent();
		result = dxApp.getAudioComponent().loadFile(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Music, L"bell0bytesIntroBark.wav"), *introMusic, audio::AudioTypes::Music);
		if (!result.isValid())
			return result;

		// send depesche to play barking sound
		core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::PlaySoundEvent, introMusic);
		dxApp.addMessage(depesche);

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::pause()
	{
		isPaused = true;

		// return success
		return { };
	}

	util::Expected<void> IntroState::resume()
	{
		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::onMessage(const core::Depesche& depesche)
	{
		input::InputHandler* ih = (input::InputHandler*)depesche.sender;

		if (!isPaused)
			if (!ih->isListening())
				return handleInput(ih->activeKeyMap);

		// return success
		return { };
	}

	util::Expected<void> IntroState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{

			case input::GameCommands::Select:
				showTradeMarkLogos = true;
				break;

			case input::GameCommands::ShowFPS:
				dxApp.toggleFPS();
				break;

			case input::GameCommands::Back:
				this->isPaused = !this->isPaused;
				break;
			}
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::update(const double deltaTime)
	{
		if (isPaused)
			return { };

		if(!showTradeMarkLogos)
			showContinueText = !showContinueText;
		else
		{
			frameTime += deltaTime;

			if (frameTime < secondsPerLogo)
				//dxApp.vibrateGamepad(15000, 15000);
				dxApp.getInputComponent().getInputHandler().vibrateGamepad(0.75f, 0.75f);

			// update trademark countdown text
			updateTrademarkCountdownTextLayout();
					
			if (frameTime > secondsPerLogo)
			{
				dxApp.getInputComponent().getInputHandler().vibrateGamepad((unsigned int)0, (unsigned int)0);
				if (!dxApp.changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to change to the main menu state!");
			}
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::render(const double /*farSeer*/)
	{
		if (!showTradeMarkLogos)
		{
			// print bell0bytes and author text
			dxApp.getGraphicsComponent().getWriteComponent().printText(0, dxApp.getGraphicsComponent().getCurrentHeight() / 2.0f - 100, companyNameLayout.Get());
			dxApp.getGraphicsComponent().getWriteComponent().printCenteredText(authorNameLayout.Get(), 30, 50);

			// blink the continue text
			if (showContinueText)
				dxApp.getGraphicsComponent().getWriteComponent().printText(0, dxApp.getGraphicsComponent().getCurrentHeight() - 100.0f, continueLayout.Get());
		}
		else
		{
			// print trademark logos of licensed products
			logos[0]->drawCentered(2, 1, 100);
			logos[1]->drawCentered(0.5f, 1, -100);
			logos[2]->drawCentered(0.4f, 700, -280);
			
			dxApp.getGraphicsComponent().getWriteComponent().printText(0, dxApp.getGraphicsComponent().getCurrentHeight() - 200.0f, trademarkLayout.Get());
			dxApp.getGraphicsComponent().getWriteComponent().printText(0, dxApp.getGraphicsComponent().getCurrentHeight() - 100.0f, trademarkCountdownLayout.Get());
		}

		// print FPS information
		dxApp.getGraphicsComponent().getWriteComponent().printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::shutdown()
	{
		// stop the barking sound
		dxApp.getAudioComponent().stopSoundEvent(*introMusic);
		
		// delete logos
		for (auto x : logos)
			delete x;

		// delete sound event
		if (introMusic)
			delete introMusic;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Helper Functions ////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::createTextFormats()
	{
		util::Expected<void> result;

		// company name
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Lucida Handwriting", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, companyNameFormat);
		if (!result.isValid())
			return result;

		// author name
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 36.0f, authorNameFormat);
		if (!result.isValid())
			return result;

		// press button to continue
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 24.0f, DWRITE_TEXT_ALIGNMENT_CENTER, continueFormat);
		if (!result.isValid())
			return result;

		// trademark countdown
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 12.0f, DWRITE_TEXT_ALIGNMENT_CENTER, trademarkCountdownFormat);
		if (!result.isValid())
			return result;

		// trademark text format
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe UI", 18.0f, DWRITE_TEXT_ALIGNMENT_CENTER, trademarkFormat);
		if (!result.isValid())
			return result;

		// return success
		return { };
	}

	util::Expected<void> IntroState::createTextLayouts()
	{
		util::Expected<void> result;

		// company and author
		std::wstring bell0bytes = L"bell0bytes presents";
		std::wstring bell0 = L"a Gilles Bellot game";

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(bell0bytes, companyNameFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, companyNameLayout);
		if (!result.isValid())
			return result;

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(bell0, authorNameFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, authorNameLayout);
		if (!result.isValid())
			return result;
		
		// press key to continue
		std::wstring continueText = L"Press 'Enter' to continue!";

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(continueText, continueFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, continueLayout);
		if (!result.isValid())
			return result;
		
		// trademark text
		std::wostringstream trademarkText;
		trademarkText << "bell0bytes tutorial " << (wchar_t)0xA9 << " bell0bytes 2018, all rights reserved - www.bell0bytes.eu" << std::endl;
		trademarkText << "DirectX 11 " << (wchar_t)0xA9 << " Microsoft 2018" << std::endl;
		trademarkText << "Boost, distributed under the Boost Software License, Version 1.0." << std::endl;
		trademarkText << "Lua (with Sol), distributed under the MIT License, Version 5.3.4" << std::endl;

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(trademarkText, trademarkFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, trademarkLayout);
		if (!result.isValid())
			return result;

		// trademark countdown text
		std::wostringstream trademarkCountdownText;
		trademarkCountdownText.precision(1);
		trademarkCountdownText << "The game will continue in: " << this->secondsPerLogo << " s." << std::endl;

		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, trademarkCountdownLayout);
		if (!result.isValid())
			return result;

		// return success
		return { };
	}

	util::Expected<void> IntroState::initializeLogoSprites()
	{
		try
		{
			// the boost logo
			logos.push_back(new graphics::Sprite(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Logos, L"logoBoost.png").c_str()));

			// the DirectX 11 logo
			logos.push_back(new graphics::Sprite(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Logos, L"logoDX11.png").c_str()));

			// the Lua logo
			logos.push_back(new graphics::Sprite(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Logos, L"logoLua.png").c_str()));
		}
		catch (std::exception&e) { return e; }
		
		// return success
		return { };
	}

	util::Expected<void> IntroState::updateTrademarkCountdownTextLayout()
	{
		std::wostringstream trademarkCountdownText;
		trademarkCountdownText.precision(1);
		double timeLeft = secondsPerLogo - frameTime;
		if (timeLeft < 0.1f)
			timeLeft = 0.0f;
		trademarkCountdownText << "The game will continue in: " << timeLeft << "s." << std::endl;
		if (!dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, trademarkCountdownLayout).wasSuccessful())
			return std::runtime_error("Critical error: Unable to create trademark countdown text layout!");

		// return success
		return { };
	}
}