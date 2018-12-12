#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		16/06/2018 - Lenningen - Luxembourg
*
* Desc:		menu buttons - lambda functions are used for button clicks
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <functional>

// bell0bytes includes
#include "expected.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace graphics
{
	class Direct2D;
	class Sprite;
	class AnimatedSprite;
}

namespace UI
{
	// all possible button states
	enum ButtonStates { Deselected, Selected, Clicked, Locked};

	class AnimatedButton
	{
	private:
		std::wstring name;							// the name of the button
		graphics::AnimatedSprite* sprite;			// the graphics of the button
		ButtonStates state;							// state of the button
		std::function<util::Expected<bool>()> onClick;// function to be replaced by a lambda function later on to define what happens when the button is clicked
		D2D1_RECT_F rect;							// the rectangle the button is drawn on
		unsigned int nAnimationCycles;				// number of animation cycles; standard: cycle 0: normal ; cycle 1: hover ; cycle 2: clicked ; cycle 3: locked
		
	public:
		// constructors and destructors
		AnimatedButton(std::wstring, graphics::AnimatedSprite* const, unsigned int clickAnimation = 2);
		AnimatedButton(std::wstring, graphics::AnimatedSprite* const, std::function<util::Expected<bool>()>, unsigned int clickAnimation = 2);
		~AnimatedButton();
		
		// button interaction
		void select();
		void deselect();
		util::Expected<bool> click();
		void lock();
		void setOnClickFunction(std::function<util::Expected<bool>()> onClickFunction);
		
		// draw button
		void draw(const float scaleFactor = 1.0f, const float offsetX = 1.0f, const float offsetY = 1.0f);
		void drawCentered(const float scaleFactor = 1.0f, const float offsetX = 1.0f, const float offsetY = 1.0f);
		
		// update button
		void update(const double deltaTime);

		// get and set button state
		const ButtonStates getButtonState() const { return state; };
		void setButtonState(ButtonStates buttonState);
		void setButtonAnimationCycle(ButtonStates buttonState);

		// get rectangle
		D2D1_RECT_F getRectangle() { return rect; };
	};
}