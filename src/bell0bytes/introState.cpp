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
	IntroState::IntroState(core::DirectXApp* const app, std::wstring name) : GameState(app, name), frameTime(0.0f), secondsPerLogo(5.0f), showContinueText(true), showTradeMarkLogos(false)
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
	void IntroState::initialize()
	{
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
		d2d->createTextFormat(L"Lucida Handwriting", 72.0f, companyNameFormat);
		d2d->createTextFormat(L"Segoe UI", 36.0f, authorNameFormat);
		d2d->createTextFormat(L"Segoe UI", 24.0f, continueFormat);
		d2d->createTextFormat(L"Segoe UI", 12.0f, trademarkCountdownFormat);
		d2d->createTextFormat(L"Segoe UI", 18.0f, trademarkFormat);

		// create text layouts
		std::wstring bell0bytes = L"bell0bytes presents";
		std::wstring bell0 = L"a Gilles Bellot game";
		std::wstring continueText = L"Press 'Enter' to continue!";
		std::wostringstream trademarkText;
		trademarkText << "bell0bytes tutorial " << (wchar_t)0xA9 << " bell0bytes 2018, all rights reserved - www.bell0bytes.eu" << std::endl;
		trademarkText << "DirectX 11 " << (wchar_t)0xA9 << " Microsoft 2018" << std::endl;
		trademarkText << "Boost, distributed under the Boost Software License, Version 1.0." << std::endl;

		d2d->createTextLayoutFromWString(&bell0bytes, companyNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, companyNameLayout);
		d2d->createTextLayoutFromWString(&bell0, authorNameFormat.Get(), (float)dxApp->getCurrentWidth(), 100, authorNameLayout);
		d2d->createTextLayoutFromWString(&continueText, continueFormat.Get(), (float)dxApp->getCurrentWidth(), 100, continueLayout);
		d2d->createTextLayoutFromWStringStream(&trademarkText, trademarkFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkLayout);

		std::wostringstream trademarkCountdownText;
		trademarkCountdownText.precision(1);
		trademarkCountdownText << "The game will continue in: " << this->secondsPerLogo << " s." << std::endl;
		d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout);

		// initialize sprites
		//boostLogo = new graphics::Sprite(d2d, L"O:/Documents/GitLab/Repositories/Symplectos/bell0tutorial/bell0tutorial/Art/Boost.png");
		//dxLogo = new graphics::Sprite(d2d, L"O:/Documents/GitLab/Repositories/Symplectos/bell0tutorial/bell0tutorial/Art/dx.png");
		boostLogo = new graphics::Sprite(d2d, L"Art/Boost.png");
		dxLogo = new graphics::Sprite(d2d, L"Art/dx.png");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void IntroState::pause()
	{
		isPaused = true;
	}

	void IntroState::resume()
	{
		// allow mouse and keyboard input
		dxApp->activeMouse = false;
		dxApp->activeKeyboard = true;

		isPaused = false;
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

	bool IntroState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{

			case input::GameCommands::Start:
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
	void IntroState::update(const double deltaTime)
	{
		if (isPaused)
			return;

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
			d2d->createTextLayoutFromWStringStream(&trademarkCountdownText, trademarkCountdownFormat.Get(), (float)dxApp->getCurrentWidth(), 100, trademarkCountdownLayout);
		
			if (frameTime > secondsPerLogo)
				dxApp->changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu"));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void IntroState::render(const double /*farSeer*/)
	{
		if (isPaused)
			return;

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
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void IntroState::shutdown()
	{
		// remove from the observer list
		dxApp->removeInputHandlerObserver(this);
		
		delete boostLogo;
		delete dxLogo;
	}
}