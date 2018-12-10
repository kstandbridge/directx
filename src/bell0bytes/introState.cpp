// INCLUDES /////////////////////////////////////////////////////////////////////////////

// bell0bytes
#include "app.h"
#include "introState.h"
#include "mainMenuState.h"
#include "gameCommands.h"
#include "inputHandler.h"
#include "sprites.h"
#include <sstream>

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	IntroState::IntroState(core::DirectXApp* const app, std::wstring name) : GameState(app, name), frameTime(0.0f), secondsPerLogo(1.0f), showContinueText(true), showTradeMarkLogos(false)
	{

	}

	IntroState::~IntroState()
	{

	}

	IntroState& IntroState::createInstance(core::DirectXApp* const app, std::wstring stateName)
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

		// allow keyboard input
		dxApp->activeMouse = false;
		dxApp->activeKeyboard = true;

		// disable fps
		//dxApp->showFPS = false;

		// create text formats
		result = d2d->createTextFormat(L"Lucida Handwriting", 72.0f, companyNameFormat);
		if (!result.isValid())
			return result;
		
		result = d2d->createTextFormat(L"Segoe UI", 36.0f, authorNameFormat);
		if (!result.isValid())
			return result;
		
		result = d2d->createTextFormat(L"Segoe UI", 24.0f, continueFormat);
		if (!result.isValid())
			return result;
		
		result = d2d->createTextFormat(L"Segoe UI", 12.0f, trademarkCountdownFormat);
		if (!result.isValid())
			return result;
		
		result = d2d->createTextFormat(L"Segoe UI", 18.0f, trademarkFormat);
		if (!result.isValid())
			return result;

		// create text layouts
		std::wstring bell0bytes = L"bell0bytes presents";
		std::wstring bell0 = L"a Gilles Bellot game";
		std::wstring continueText = L"Press 'Enter' to continue!";
		std::wostringstream trademarkText;
		trademarkText << "bell0bytes tutorial " << (wchar_t)0xA9 << " bell0bytes 2018, all rights reserved - www.bell0bytes.eu" << std::endl;
		trademarkText << "DirectX 11 " << (wchar_t)0xA9 << " Microsoft 2018" << std::endl;
		trademarkText << "Boost, distributed under the Boost Software License, Version 1.0." << std::endl;

		result = d2d->createTextLayoutFromWString(&bell0bytes, companyNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, companyNameLayout);
		if (!result.isValid())
			return result;

		result = d2d->createTextLayoutFromWString(&bell0, authorNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, authorNameLayout);
		if (!result.isValid())
			return result;
		
		result = d2d->createTextLayoutFromWString(&continueText, continueFormat.Get(), (float)dxApp->getCurrentWidth(), 100, continueLayout);
		if (!result.isValid())
			return result; 
		
		result = d2d->createTextLayoutFromWStringStream(&trademarkText, trademarkFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkLayout);
		if (!result.isValid())
			return result;

		std::wostringstream trademarkCountdownText;
		trademarkCountdownText.precision(1);
		trademarkCountdownText << "The game will continue in: " << this->secondsPerLogo << " s." << std::endl;
		result = d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout);
		if (!result.isValid())
			return result;

		// initialize sprites
		boostLogo = new graphics::Sprite(d2d, L"Art/Boost.png");
		dxLogo = new graphics::Sprite(d2d, L"Art/dx.png");

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
		// allow mouse and keyboard input
		dxApp->activeMouse = false;
		dxApp->activeKeyboard = true;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<bool> IntroState::onNotify(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		if (!isPaused)
			return handleInput(activeKeyMap);

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
				dxApp->showFPS = !dxApp->showFPS;
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
			
			std::wostringstream trademarkCountdownText;
			trademarkCountdownText.precision(1);
			double timeLeft = this->secondsPerLogo - this->frameTime;
			if (timeLeft < 0.1f)
				timeLeft = 0.0f;
			trademarkCountdownText << "The game will continue in: " << timeLeft << "s." << std::endl;
			if (!d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout).wasSuccessful())
				return std::runtime_error("Critical error: Unable to create trademark countdown text layout!");
		
			if (frameTime > secondsPerLogo)
				dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu"));
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> IntroState::render(const double /*farSeer*/)
	{
		if (isPaused)
			return { };

		if (!showTradeMarkLogos)
		{
			// print bell0bytes text
			d2d->printCenteredText(companyNameLayout.Get());
			d2d->printCenteredText(authorNameLayout.Get(), 5.5f, 50);
			
			if (showContinueText)
				d2d->printCenteredText(continueLayout.Get(), 0, dxApp->getCurrentHeight() / 2.0f - 25.0f);
		}
		else
		{
			// print trademark logos of licensed products
			boostLogo->drawCentered(2);
			dxLogo->drawCentered(0.5, 1, -200);
			
			d2d->printCenteredText(trademarkLayout.Get(), -150, (dxApp->getCurrentHeight()/2.0f) - 50.0f);
			d2d->printCenteredText(trademarkCountdownLayout.Get(), -75, (dxApp->getCurrentHeight()/2.0f));
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
		
		delete boostLogo;
		delete dxLogo;

		// return success
		return { };
	}
}