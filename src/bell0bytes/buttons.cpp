// INCLUDES /////////////////////////////////////////////////////////////////////////////

// directx includes
#include <d2d1_3.h>

// c++ includes
#include <functional>

// bell0bytes
#include "buttons.h"
#include "sprites.h"


// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	AnimatedButton::AnimatedButton(std::wstring name, graphics::AnimatedSprite* const sprite, unsigned int nAnimations) : name(name), sprite(sprite), state(ButtonStates::Deselected), nAnimationCycles(nAnimations)
	{

	}

	AnimatedButton::AnimatedButton(std::wstring name, graphics::AnimatedSprite* const sprite, std::function<util::Expected<bool>()> onClick, unsigned int nAnimations) : name(name), sprite(sprite), onClick(onClick), state(ButtonStates::Deselected), nAnimationCycles(nAnimations)
	{

	}

	AnimatedButton::~AnimatedButton()
	{
		onClick = []() { return true; };
		delete this->sprite;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Drawing ///////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AnimatedButton::draw(const float scaleFactor, const float offsetX, const float offsetY)
	{
		this->sprite->draw(scaleFactor, offsetX, offsetY, &rect);
	}

	void AnimatedButton::drawCentered(const float scaleFactor, const float offsetX, const float offsetY)
	{
		sprite->drawCentered(scaleFactor, offsetX, offsetY, &rect);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Interaction ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AnimatedButton::select()
	{
		state = ButtonStates::Selected;
		sprite->changeAnimation(1);
	}

	void AnimatedButton::deselect()
	{
		state = ButtonStates::Deselected;
		sprite->changeAnimation(0);
	}

	util::Expected<bool> AnimatedButton::click()
	{
		if (state == ButtonStates::Locked)
			return true;

		if (nAnimationCycles > 2)
			sprite->changeAnimation(2);
		state = ButtonStates::Clicked;
		return onClick();
	}

	void AnimatedButton::lock()
	{
		if (nAnimationCycles > 3)
			sprite->changeAnimation(3);
		state = ButtonStates::Locked;
	}

	void AnimatedButton::setOnClickFunction(std::function<util::Expected<bool>()> onClickFunction)
	{
		onClick = onClickFunction;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Update ///////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AnimatedButton::update(const double deltaTime)
	{
		sprite->updateAnimation(deltaTime);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Getters and Setters ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AnimatedButton::setButtonState(ButtonStates buttonState)
	{
		switch (buttonState)
		{
		case ButtonStates::Deselected:
			this->deselect();
			break;

		case ButtonStates::Selected:
			this->select();
			break;

		case ButtonStates::Clicked:
			this->click();
			break;

		case ButtonStates::Locked:
			this->lock();
			break;
		}
	}

	void AnimatedButton::setButtonAnimationCycle(ButtonStates buttonState)
	{
		switch (buttonState)
		{
		case ButtonStates::Deselected:
			this->deselect();
			break;

		case ButtonStates::Selected:
			this->select();
			break;

		case ButtonStates::Clicked:
			if (nAnimationCycles > 2)
				sprite->changeAnimation(2);
			state = ButtonStates::Clicked;
			break;

		case ButtonStates::Locked:
			this->lock();
			break;
		}
	}
}