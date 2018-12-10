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
	class AnimatedSprite;
}

namespace UI
{
	// all possible button states
	enum ButtonStates { Deselected, Selected, Clicked};

	class Button
	{
	private:
		std::wstring name;							// the name of the button
		graphics::AnimatedSprite* sprite;			// the graphics of the button
		ButtonStates state;							// state of the button
		std::function<bool()> onClick;				// function to be replaced by a lambda function later on to define what happens when the button is clicked
		D2D1_RECT_F rect;							// the rectangle the button is drawn on

	public:
		// constructors and destructors
		Button(std::wstring, graphics::AnimatedSprite* const);
		Button(std::wstring, graphics::AnimatedSprite* const, std::function<bool()>);
		~Button();
		
		// button interaction
		void select();
		void deselect();
		bool click();
		void setOnClickFunction(std::function<bool()> onClickFunction);
		
		// draw button
		void draw();
		void drawCentered(const float scaleFactor = 1.0f, const float offsetX = 1.0f, const float offsetY = 1.0f);
		
		// update button
		void update(const double deltaTime);

		// get rectangle
		D2D1_RECT_F getRectangle() { return rect; };
	};
}