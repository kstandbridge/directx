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
#include "inputHandler.h"		// the input handler

// bell0bytes graphics
#include "sprites.h"			// sprites for the logos

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	IntroState::IntroState(core::DirectXApp* const app, const std::wstring& name) : GameState(app, name), frameTime(0.0f), secondsPerLogo(5.0f), showContinueText(true), showTradeMarkLogos(false)
	{ }

	IntroState::~IntroState()
	{ }

	IntroState& IntroState::createInstance(core::DirectXApp* const app, const std::wstring& stateName)
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

		// add to observer list of the input handler
		dxApp->addInputHandlerObserver(this);

		// hide the standard cursor
		ShowCursor(false);

		// allow keyboard input and disable mouse input
		dxApp->activeMouse = false;
		dxApp->activeKeyboard = true;
		
		// create text formats
		if (!createTextFormats().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text formats!");
		
		// create text layouts
		if (!createTextLayouts().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create text layouts!");

		// create sprites
		if (!initializeLogoSprites().wasSuccessful())
			return std::runtime_error("Critical error: Unable to initialize logo sprites!");
			
		// return success
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
	util::Expected<bool> IntroState::onNotify(input::InputHandler* const ih, const bool listening)
	{
		if(!listening)
			return handleInput(ih->activeKeyMap);

		// return success
		return true;
	}

	util::Expected<bool> IntroState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
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
				dxApp->toggleFPS();
				break;

			case input::GameCommands::Back:
				this->isPaused = !this->isPaused;
				break;
			}
		}

		return true;
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
			
			// update trademark countdown text
			updateTrademarkCountdownTextLayout();
					
			if (frameTime > secondsPerLogo)
				if (!dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
					return std::runtime_error("Critical error: Unable to change to the main menu state!");
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
			d2d->printText(0, dxApp->getCurrentHeight() / 2.0f - 100, companyNameLayout.Get());
			d2d->printCenteredText(authorNameLayout.Get(), 30, 50);

			// blink the continue text
			if (showContinueText)
				d2d->printText(0, dxApp->getCurrentHeight() - 100.0f, continueLayout.Get());
		}
		else
		{
			// print trademark logos of licensed products
			logos[0]->drawCentered(2, 1, 100);
			logos[1]->drawCentered(0.5f, 1, -100);
			logos[2]->drawCentered(0.4f, 700, -280);
			
			d2d->printText(0, dxApp->getCurrentHeight() - 200.0f, trademarkLayout.Get());
			d2d->printText(0, dxApp->getCurrentHeight() - 100.0f, trademarkCountdownLayout.Get());
		}

		// print FPS information
		d2d->printFPS();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::shutdown()
	{
		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);
		
		// delete logos
		for (auto x : logos)
			delete x;

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
		result = d2d->createTextFormat(L"Lucida Handwriting", 72.0f, DWRITE_TEXT_ALIGNMENT_CENTER, companyNameFormat);
		if (!result.isValid())
			return result;

		// author name
		result = d2d->createTextFormat(L"Segoe UI", 36.0f, authorNameFormat);
		if (!result.isValid())
			return result;

		// press button to continue
		result = d2d->createTextFormat(L"Segoe UI", 24.0f, DWRITE_TEXT_ALIGNMENT_CENTER, continueFormat);
		if (!result.isValid())
			return result;

		// trademark countdown
		result = d2d->createTextFormat(L"Segoe UI", 12.0f, DWRITE_TEXT_ALIGNMENT_CENTER, trademarkCountdownFormat);
		if (!result.isValid())
			return result;

		// trademark text format
		result = d2d->createTextFormat(L"Segoe UI", 18.0f, DWRITE_TEXT_ALIGNMENT_CENTER, trademarkFormat);
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

		result = d2d->createTextLayoutFromWString(&bell0bytes, companyNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, companyNameLayout);
		if (!result.isValid())
			return result;

		result = d2d->createTextLayoutFromWString(&bell0, authorNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, authorNameLayout);
		if (!result.isValid())
			return result;
		
		// press key to continue
		std::wstring continueText = L"Press 'Enter' to continue!";

		result = d2d->createTextLayoutFromWString(&continueText, continueFormat.Get(), (float)dxApp->getCurrentWidth(), 100, continueLayout);
		if (!result.isValid())
			return result;
		
		// trademark text
		std::wostringstream trademarkText;
		trademarkText << "bell0bytes tutorial " << (wchar_t)0xA9 << " bell0bytes 2018, all rights reserved - www.bell0bytes.eu" << std::endl;
		trademarkText << "DirectX 11 " << (wchar_t)0xA9 << " Microsoft 2018" << std::endl;
		trademarkText << "Boost, distributed under the Boost Software License, Version 1.0." << std::endl;
		trademarkText << "Lua (with Sol), distributed under the MIT License, Version 5.3.4" << std::endl;

		result = d2d->createTextLayoutFromWStringStream(&trademarkText, trademarkFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkLayout);
		if (!result.isValid())
			return result;

		// trademark countdown text
		std::wostringstream trademarkCountdownText;
		trademarkCountdownText.precision(1);
		trademarkCountdownText << "The game will continue in: " << this->secondsPerLogo << " s." << std::endl;

		result = d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout);
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
			logos.push_back(new graphics::Sprite(d2d, dxApp->openFile(fileSystem::DataFolders::Logos, L"logoBoost.png").c_str()));

			// the DirectX 11 logo
			logos.push_back(new graphics::Sprite(d2d, dxApp->openFile(fileSystem::DataFolders::Logos, L"logoDX11.png").c_str()));

			// the Lua logo
			logos.push_back(new graphics::Sprite(d2d, dxApp->openFile(fileSystem::DataFolders::Logos, L"logoLua.png").c_str()));
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
		if (!d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout).wasSuccessful())
			return std::runtime_error("Critical error: Unable to create trademark countdown text layout!");

		// return success
		return { };
	}
}