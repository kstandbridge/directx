#include "sprites.h"
#include "serviceLocator.h"
#include "d2d.h"
#include "serviceLocator.h"

#include <wincodec.h>	// Windows Imaging Component
#include <WTypes.h>

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Sprite::Sprite(Direct2D* const d2d, LPCWSTR imageFile, const float x, const float y, const Layers layer, const unsigned int drawOrder) : d2d(d2d), x(x), y(y), layer(layer), drawOrder(drawOrder)
	{
		d2d->createBitmapFromWICBitmap(imageFile, bitmap);
		size = bitmap->GetSize();
	}

	Sprite::Sprite(Direct2D* const d2d, ID2D1Bitmap1* const bitmap, const float x, const float y, const Layers layer, const unsigned int drawOrder) : d2d(d2d), bitmap(bitmap), x(x), y(y), layer(layer), drawOrder(drawOrder)
	{
		size = bitmap->GetSize();
	}

	AnimationData::AnimationData(Direct2D* const d2d, LPCWSTR spriteSheetFile, const std::vector<AnimationCycleData>& cyclesData) : cyclesData(cyclesData)
	{
		d2d->createBitmapFromWICBitmap(spriteSheetFile, spriteSheet);
	}

	SpriteMap::SpriteMap()
	{

	}

	AnimatedSprite::AnimatedSprite(Direct2D* const d2d, AnimationData* const animationData, const unsigned int activeAnimation, const float animationFPS, const float x, const float y, const Layers layer, const unsigned int drawOrder) : Sprite(d2d, animationData->spriteSheet.Get(), x, y, layer, drawOrder), animationData(animationData), activeAnimation(activeAnimation), animationFPS(animationFPS), activeAnimationFrame(0)
	{

	}
	
	Sprite::~Sprite()
	{
		// release bitmap
		this->bitmap.ReleaseAndGetAddressOf();
	}

	SpriteMap::~SpriteMap()
	{
		for (auto sprite : backgroundMap)
			delete sprite.second;
		for (auto sprite : characterMap)
			delete sprite.second;
		for (auto sprite : userInterfaceMap)
			delete sprite.second;
	}

	AnimationData::~AnimationData()
	{
		// clear the vector
		this->cyclesData.clear();
		std::vector<AnimationCycleData>(cyclesData).swap(cyclesData);

		// release the sprite sheet, to be sure
		spriteSheet.ReleaseAndGetAddressOf();
	}

	AnimatedSprite::~AnimatedSprite()
	{
		delete animationData;// = nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Populating //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void SpriteMap::addSprite(Sprite* const sprite)
	{
		Layers layer = sprite->layer;
		switch (layer)
		{
		case Layers::Background:
			backgroundMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite->drawOrder), std::move(sprite)));
			break;

		case Layers::Characters:
			characterMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite->drawOrder), std::move(sprite)));
			break;

		case Layers::UserInterface:
			userInterfaceMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite->drawOrder), std::move(sprite)));
			break;
		}
	}

	util::Expected<void> SpriteMap::addSprite(Direct2D* const d2d, LPCWSTR imageFile, const float x, const float y, const Layers layer, unsigned int drawOrder)
	{
		try
		{
			Layers l = layer;

			switch (l)
			{
			case Layers::Background:
				backgroundMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(drawOrder), new Sprite(d2d, imageFile, x, y, layer, drawOrder)));
				break;

			case Layers::Characters:
				characterMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(drawOrder), new Sprite(d2d, imageFile, x, y, layer, drawOrder)));
				break;

			case Layers::UserInterface:
				userInterfaceMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(drawOrder), new Sprite(d2d, imageFile, x, y, layer, drawOrder)));
				break;
			}
		}
		catch (std::runtime_error& e)
		{
			return e;
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Drawing //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	D2D1_RECT_F Sprite::getCenteredRectangle(const float scaleFactor) const
	{
		float centerWidth = (float)d2d->getCurrentWidth() / 2.0f;
		float centerHeight = (float)d2d->getCurrentHeight() / 2.0f;
		return { centerWidth - scaleFactor * (this->size.width / 2.0f), centerHeight - scaleFactor * (this->size.height / 2.0f), centerWidth + scaleFactor * (this->size.width / 2.0f), centerHeight + scaleFactor * (this->size.height / 2.0f) };
	}

	void Sprite::draw(const D2D1_RECT_F* const destRect, const D2D1_RECT_F* const sourceRect, const float opacity, const D2D1_BITMAP_INTERPOLATION_MODE interPol) const
	{
		if (!destRect)
		{
			// set suitable destination rectangle
			D2D1_RECT_F rect = { this->x, this->y, this->x + size.width, this->y + size.height };

			d2d->devCon->DrawBitmap(bitmap.Get(), rect, opacity, interPol, sourceRect);
		}
		else
			d2d->devCon->DrawBitmap(bitmap.Get(), destRect, opacity, interPol, sourceRect);
	}

	void Sprite::drawCentered(const float scaleFactor, const float xOffset, const float yOffset, const float opacity, D2D1_BITMAP_INTERPOLATION_MODE interPol, const D2D1_RECT_F* const sourceRect) const
	{
		// set suitable destination rectangle
		D2D1_RECT_F rect = getCenteredRectangle(scaleFactor);
		rect.left += xOffset;
		rect.right += xOffset;
		rect.top += yOffset;
		rect.bottom += yOffset;

		d2d->devCon->DrawBitmap(bitmap.Get(), rect, opacity, interPol, sourceRect);
	}

	void AnimatedSprite::draw() const
	{
		unsigned int cycle = this->activeAnimation;
		unsigned int frame = this->activeAnimationFrame;
		AnimationCycleData cycleData = animationData->cyclesData[cycle];

		// compute the destination rectangle, make sure to put the rotation centre at the desired position
		// upper left x: x + width * rotationCenterX
		// upper left y: y + height * rotationCenterY
		// lower right x: x + width * (1-rotationCenterX)
		// lower right y: y + height * (1-rotationCenterY)

		D2D1_RECT_F destRect = {this->x-(cycleData.width*cycleData.rotationCenterX), 
								this->y-(cycleData.height*cycleData.rotationCenterY), 
								this->x+(cycleData.width*(1.0f-cycleData.rotationCenterX)),
								this->y+(cycleData.height*(1.0f-cycleData.rotationCenterY))};

		// compute the coordinates of the upper left corner of the source rectangle
		// upper left x of the i-th sprite: border padding plus i-times the combined width of each image and padding between images
		// upper left y of the i-th sprite: border padding plus the combined heights of each image and padding between images in the previous cycles
		float startX = frame * (cycleData.width + cycleData.paddingWidth) + cycleData.borderPaddingWidth;
		float startY = 0;
		for (unsigned int i = 0; i < cycle; i++)
			startY += (animationData->cyclesData[i].height + animationData->cyclesData[i].paddingHeight);
		startY += animationData->cyclesData[0].borderPaddingHeight;

		// create the source rectangle
		// obviously to get the lower right coordinates, we simply have to add the width and height respectively
		D2D1_RECT_F sourceRect = { startX, startY, startX + cycleData.width, startY + cycleData.height };

		Sprite::draw(&destRect, &sourceRect);
	}

	void SpriteMap::draw(D2D1_RECT_F* const destRect, D2D1_RECT_F* const sourceRect, const DrawCommands drawCommand, const float opacity, const D2D1_BITMAP_INTERPOLATION_MODE interPol) const
	{
		D2D1_RECT_F* rect;
		if (destRect)
			rect = destRect;
		else
			rect = NULL;

		switch (drawCommand)
		{
		case DrawCommands::All:
			// draw the background first
			for (auto sprite : backgroundMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);

			// draw the characters
			for (auto sprite : characterMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);

			// draw the user interface
			for (auto sprite : userInterfaceMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);

			break;

		case DrawCommands::onlyBackground:
			// draw the background first
			for (auto sprite : backgroundMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);
			break;

		case DrawCommands::onlyCharacters:
			// draw the background first
			for (auto sprite : characterMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);
			break;

		case DrawCommands::onlyUserInterface:
			// draw the background first
			for (auto sprite : userInterfaceMap)
				sprite.second->draw(rect, sourceRect, opacity, interPol);
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Updating /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AnimatedSprite::changeAnimation(const unsigned int cycleToActivate)
	{
		if (cycleToActivate > this->animationData->cyclesData.size())
		{
			// print error
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Unable to activate the desired animation cycle! Reverting to the default cycle!");

			// activate the first animation cycle
			activeAnimation = 0;
			activeAnimationFrame = 0;
			frameTime = 0.0f;
		}
		else
		{
			// activate the first frame of the new animation and set the active time to 0
			activeAnimation = cycleToActivate;
			activeAnimationFrame = 0;
			frameTime = 0.0f;
		}
	}

	void AnimatedSprite::updateAnimation(const double deltaTime)
	{
		// update how long the currently active frame has been displayed
		frameTime += deltaTime;

		// check whether it is time to change to another frame
		if (frameTime > (1.0f / (double)animationFPS))
		{
			// the number of frames to increment is the integral result of frameTime / (1 / animationFPS), thus frameTime * animationFPS
			activeAnimationFrame += (unsigned int)(frameTime * animationFPS);

			// use modulo computation to make sure to not jump past the last frame
			if (activeAnimationFrame >= animationData->cyclesData[activeAnimation].numberOfFrames)
				activeAnimationFrame = activeAnimationFrame % animationData->cyclesData[activeAnimation].numberOfFrames;
		}

		// set the frame time
		frameTime = std::fmod(frameTime, 1.0f / (double)animationFPS);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////// Getters and Setters //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void Sprite::setPosition(float posX, float posY)
	{
		this->x = posX;
		this->y = posY;
	}
}