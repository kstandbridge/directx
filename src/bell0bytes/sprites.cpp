// header
#include "sprites.h"

// bell0bytes graphics
#include "d2d.h"

// bell0bytes util
#include "serviceLocator.h"
#include "expected.h"

namespace graphics
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Sprite::Sprite(const Direct2D& d2d, LPCWSTR imageFile, const float x, const float y, const Layers layer, const unsigned int drawOrder) : d2d(d2d), x(x), y(y), layer(layer), drawOrder(drawOrder)
	{
		d2d.createBitmapFromWICBitmap(imageFile, bitmap);

		if (bitmap == nullptr)
			throw std::runtime_error("Critical error: failed to create the sprite bitmap!");

		size = bitmap->GetSize();
	}
	Sprite::Sprite(const Direct2D& d2d, const float x, const float y, const Layers layer, const unsigned int drawOrder) : d2d(d2d), bitmap(bitmap), x(x), y(y), layer(layer), drawOrder(drawOrder)
	{ }
	Sprite::Sprite(const Direct2D& d2d, ID2D1Bitmap1* const bitmap, const float x, const float y, const Layers layer, const unsigned int drawOrder) : d2d(d2d), bitmap(bitmap), x(x), y(y), layer(layer), drawOrder(drawOrder)
	{
		if (bitmap == nullptr)
			throw std::runtime_error("Critical error: bitmap passed to the Sprite constructor was a nullptr!");

		size = bitmap->GetSize();
	}
	AnimationData::AnimationData(const Direct2D& d2d, LPCWSTR spriteSheetFile, const std::vector<AnimationCycleData>& cyclesData) : cyclesData(cyclesData)
	{
		d2d.createBitmapFromWICBitmap(spriteSheetFile, spriteSheet);

		if (spriteSheet == nullptr)
			throw std::runtime_error("Critical error: Unable to create sprite sheet from file!");
	}
	AnimationData::AnimationData(const Direct2D& d2d, LPCWSTR spriteSheetFile, const AnimationCycleData& cycleData)
	{
		cyclesData.clear();
		cyclesData.push_back(cycleData);

		d2d.createBitmapFromWICBitmap(spriteSheetFile, spriteSheet);

		if (spriteSheet == nullptr)
			throw std::runtime_error("Critical error: Unable to create sprite sheet from file!");
	}
	SpriteMap::SpriteMap()
	{

	}

	AnimatedSprite::AnimatedSprite(const Direct2D& d2d, const unsigned int activeAnimation, const float animationFPS, const float x, const float y, const Layers layer, const unsigned int drawOrder) : Sprite(d2d, x, y, layer, drawOrder), activeAnimation(activeAnimation), animationFPS(animationFPS), activeAnimationFrame(0)
	{
		animationData.clear();
	}

	AnimatedSprite::AnimatedSprite(const Direct2D& d2d, AnimationData* const animationData, const unsigned int activeAnimation, const float animationFPS, const float x, const float y, const Layers layer, const unsigned int drawOrder) : Sprite(d2d, animationData->spriteSheet.Get(), x, y, layer, drawOrder), activeAnimation(activeAnimation), animationFPS(animationFPS), activeAnimationFrame(0)
	{
		if (!animationData)
			throw std::runtime_error("Critical error: the animation data passed to the animated sprite was empty!");

		this->animationData.push_back(animationData);
	}

	// activates the first sprite sheet
	AnimatedSprite::AnimatedSprite(const Direct2D& d2d, const std::vector<AnimationData*>& animationData, const unsigned int activeAnimation, const float animationFPS, const float x, const float y, const Layers layer, const unsigned int drawOrder) : Sprite(d2d, animationData[0]->spriteSheet.Get(), x, y, layer, drawOrder), animationData(animationData), activeAnimation(activeAnimation), animationFPS(animationFPS), activeAnimationFrame(0)
	{ }

	// adds an animation
	void AnimatedSprite::addAnimation(AnimationData* const animData, bool updateSprite)
	{
		animationData.push_back(animData);

		if (updateSprite)
			bitmap = animData->spriteSheet;
	}

	Sprite::~Sprite()
	{
		// release bitmap
		if(this->bitmap.Get() != nullptr)
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
		Sprite::~Sprite();

		for(auto animData : animationData)
			delete animData;
		animationData.clear();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Populating //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void SpriteMap::addSprite(Sprite& sprite)
	{
		Layers layer = sprite.layer;
		switch (layer)
		{
		case Layers::Background:
			backgroundMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite.drawOrder), std::move(&sprite)));
			break;

		case Layers::Characters:
			characterMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite.drawOrder), std::move(&sprite)));
			break;

		case Layers::UserInput:
			userInterfaceMap.insert(std::make_pair<unsigned int, Sprite*const>(std::move(sprite.drawOrder), std::move(&sprite)));
			break;
		}
	}
	util::Expected<void> SpriteMap::addSprite(const Direct2D& d2d, LPCWSTR imageFile, const float x, const float y, const Layers layer, unsigned int drawOrder)
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

			case Layers::UserInput:
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
		float centerWidth = (float)d2d.getCurrentWidth() / 2.0f;
		float centerHeight = (float)d2d.getCurrentHeight() / 2.0f;
		return { centerWidth - scaleFactor * (this->size.width / 2.0f), centerHeight - scaleFactor * (this->size.height / 2.0f), centerWidth + scaleFactor * (this->size.width / 2.0f), centerHeight + scaleFactor * (this->size.height / 2.0f) };
	}
	void Sprite::draw(const D2D1_RECT_F* const destRect, const D2D1_RECT_F* const sourceRect, const float opacity, const D2D1_BITMAP_INTERPOLATION_MODE interPol) const
	{
		if (!destRect)
		{
			// set suitable destination rectangle
			D2D1_RECT_F rect = { this->x, this->y, this->x + size.width, this->y + size.height };

			d2d.devCon->DrawBitmap(bitmap.Get(), rect, opacity, interPol, sourceRect);
		}
		else
			d2d.devCon->DrawBitmap(bitmap.Get(), destRect, opacity, interPol, sourceRect);
	}
	void Sprite::drawCentered(const float scaleFactor, const float xOffset, const float yOffset, const float opacity, D2D1_BITMAP_INTERPOLATION_MODE interPol, const D2D1_RECT_F* const sourceRect) const
	{
		// set suitable destination rectangle
		D2D1_RECT_F rect = getCenteredRectangle(scaleFactor);
		rect.left += xOffset;
		rect.right += xOffset;
		rect.top += yOffset;
		rect.bottom += yOffset;

		d2d.devCon->DrawBitmap(bitmap.Get(), rect, opacity, interPol, sourceRect);
	}
	void AnimatedSprite::draw(const float scaleFactor, const float offsetX, const float offsetY, D2D1_RECT_F* const rect) const
	{
		// rectangles
		D2D1_RECT_F destRect, sourceRect;

		if (animationData.size() == 1)
		{
			unsigned int cycle = this->activeAnimation;
			unsigned int frame = this->activeAnimationFrame;
			AnimationCycleData cycleData = animationData[0]->cyclesData[cycle];

			// compute the destination rectangle, make sure to put the rotation centre at the desired position
			// upper left x: x + offsetX - width * rotationCenterX * scaleFactor
			// upper left y: y + offsetY - height * rotationCenterY * scaleFactor
			// lower right x: x + offsetX + width * (1-rotationCenterX) * scaleFactor
			// lower right y: y + offsetY + height * (1-rotationCenterY) * scaleFactor

			destRect = { this->x + offsetX - (cycleData.width*cycleData.rotationCenterX*scaleFactor),
									this->y + offsetY - (cycleData.height*cycleData.rotationCenterY*scaleFactor),
									this->x + offsetX + (cycleData.width*(1.0f - cycleData.rotationCenterX)*scaleFactor),
									this->y + offsetY + (cycleData.height*(1.0f - cycleData.rotationCenterY)*scaleFactor) };

			if (rect != NULL)
				*rect = destRect;

			// compute the coordinates of the upper left corner of the source rectangle
			// upper left x of the i-th sprite: border padding plus i-times the combined width of each image and padding between images
			// upper left y of the i-th sprite: border padding plus the combined heights of each image and padding between images in the previous cycles
			float startX = frame * (cycleData.width + cycleData.paddingWidth) + cycleData.borderPaddingWidth;
			float startY = 0;
			for (unsigned int i = 0; i < cycle; i++)
				startY += (animationData[0]->cyclesData[i].height + animationData[0]->cyclesData[i].paddingHeight);
			startY += animationData[0]->cyclesData[0].borderPaddingHeight;

			// create the source rectangle
			// obviously to get the lower right coordinates, we simply have to add the width and height respectively
			sourceRect = { startX, startY, startX + cycleData.width, startY + cycleData.height };
		}
		else
		{
			unsigned int cycle = this->activeAnimation;
			unsigned int frame = this->activeAnimationFrame;
			AnimationCycleData cycleData = animationData[cycle]->cyclesData[0];

			// compute the destination rectangle, make sure to put the rotation centre at the desired position
			// upper left x: x + offsetX - width * rotationCenterX * scaleFactor
			// upper left y: y + offsetY - height * rotationCenterY * scaleFactor
			// lower right x: x + offsetX + width * (1-rotationCenterX) * scaleFactor
			// lower right y: y + offsetY + height * (1-rotationCenterY) * scaleFactor

			destRect = { this->x + offsetX - (cycleData.width*cycleData.rotationCenterX*scaleFactor),
				this->y + offsetY - (cycleData.height*cycleData.rotationCenterY*scaleFactor),
				this->x + offsetX + (cycleData.width*(1.0f - cycleData.rotationCenterX)*scaleFactor),
				this->y + offsetY + (cycleData.height*(1.0f - cycleData.rotationCenterY)*scaleFactor) };

			if (rect != NULL)
				*rect = destRect;

			// compute the coordinates of the upper left corner of the source rectangle
			// upper left x of the i-th sprite: border padding plus i-times the combined width of each image and padding between images
			// upper left y of the i-th sprite: border padding
			float startX = frame * (cycleData.width + cycleData.paddingWidth) + cycleData.borderPaddingWidth;
			float startY = animationData[cycle]->cyclesData[0].borderPaddingHeight;

			// create the source rectangle
			// obviously to get the lower right coordinates, we simply have to add the width and height respectively
			sourceRect = { startX, startY, startX + cycleData.width, startY + cycleData.height };
		}

		Sprite::draw(&destRect, &sourceRect);
	}
	
	void AnimatedSprite::drawCentered(const float scaleFactor, const float offsetX, const float offsetY, D2D1_RECT_F* const rect) const
	{
		D2D1_RECT_F sourceRect, destRect;

		if (animationData.size() == 1)
		{
			unsigned int cycle = this->activeAnimation;
			unsigned int frame = this->activeAnimationFrame;
			AnimationCycleData cycleData = animationData[0]->cyclesData[cycle];

			// get screen center
			float centerX = d2d.getCurrentWidth() / 2.0f;
			float centerY = d2d.getCurrentHeight() / 2.0f;

			// compute the destination rectangle, make sure to put the rotation centre at the desired position
			// upper left x: centerX + offsetX + width * rotationCenterX * scaleFactor
			// upper left y: centerY + offsetY + height * rotationCenterY * scaleFactor
			// lower right x: centerX + offsetX + width * (1-rotationCenterX) * scaleFactor
			// lower right y: centerY + offsetY + height * (1-rotationCenterY) * scaleFactor

			destRect = { offsetX + centerX - (cycleData.width*cycleData.rotationCenterX * scaleFactor),
				offsetY + centerY - (cycleData.height*cycleData.rotationCenterY * scaleFactor),
				offsetX + centerX + (cycleData.width*(1.0f - cycleData.rotationCenterX) * scaleFactor),
				offsetY + centerY + (cycleData.height*(1.0f - cycleData.rotationCenterY) * scaleFactor) };

			if (rect != NULL)
				*rect = destRect;

			// compute the coordinates of the upper left corner of the source rectangle
			// upper left x of the i-th sprite: border padding plus i-times the combined width of each image and padding between images
			// upper left y of the i-th sprite: border padding plus the combined heights of each image and padding between images in the previous cycles
			float startX = frame * (cycleData.width + cycleData.paddingWidth) + cycleData.borderPaddingWidth;
			float startY = 0;
			for (unsigned int i = 0; i < cycle; i++)
				startY += (animationData[0]->cyclesData[i].height + animationData[0]->cyclesData[i].paddingHeight);
			startY += animationData[0]->cyclesData[0].borderPaddingHeight;

			// create the source rectangle
			// obviously to get the lower right coordinates, we simply have to add the width and height respectively
			sourceRect = { startX, startY, startX + cycleData.width, startY + cycleData.height };
		}
		else
		{
			unsigned int cycle = this->activeAnimation;
			unsigned int frame = this->activeAnimationFrame;
			AnimationCycleData cycleData = animationData[cycle]->cyclesData[0];

			// get screen center
			float centerX = d2d.getCurrentWidth() / 2.0f;
			float centerY = d2d.getCurrentHeight() / 2.0f;

			// compute the destination rectangle, make sure to put the rotation centre at the desired position
			// upper left x: centerX + offsetX + width * rotationCenterX * scaleFactor
			// upper left y: centerY + offsetY + height * rotationCenterY * scaleFactor
			// lower right x: centerX + offsetX + width * (1-rotationCenterX) * scaleFactor
			// lower right y: centerY + offsetY + height * (1-rotationCenterY) * scaleFactor

			destRect = { offsetX + centerX - (cycleData.width*cycleData.rotationCenterX * scaleFactor),
				offsetY + centerY - (cycleData.height*cycleData.rotationCenterY * scaleFactor),
				offsetX + centerX + (cycleData.width*(1.0f - cycleData.rotationCenterX) * scaleFactor),
				offsetY + centerY + (cycleData.height*(1.0f - cycleData.rotationCenterY) * scaleFactor) };

			if (rect != NULL)
				*rect = destRect;

			// compute the coordinates of the upper left corner of the source rectangle
			// upper left x of the i-th sprite: border padding plus i-times the combined width of each image and padding between images
			// upper left y of the i-th sprite: border padding plus the combined heights of each image and padding between images in the previous cycles
			float startX = frame * (cycleData.width + cycleData.paddingWidth) + cycleData.borderPaddingWidth;
			float startY = animationData[cycle]->cyclesData[0].borderPaddingHeight;

			// create the source rectangle
			// obviously to get the lower right coordinates, we simply have to add the width and height respectively
			sourceRect = { startX, startY, startX + cycleData.width, startY + cycleData.height };
		}

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
		if (animationData.size() == 1)
		{
			if (cycleToActivate > this->animationData[0]->cyclesData.size())
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
		else
		{
			if (cycleToActivate > this->animationData.size())
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

			// set the sprite bitmap
			bitmap.ReleaseAndGetAddressOf();
			bitmap = this->animationData[activeAnimation]->spriteSheet;
		}
	}
	
	void AnimatedSprite::updateAnimation(const double deltaTime, const bool loop)
	{
		// update how long the currently active frame has been displayed
		frameTime += deltaTime;

		// check whether it is time to change to another frame
		if (frameTime > (1.0f / (double)animationFPS))
		{
			// the number of frames to increment is the integral result of frameTime / (1 / animationFPS), thus frameTime * animationFPS
			activeAnimationFrame += (unsigned int)(frameTime * animationFPS);

			if (animationData.size() == 1)
			{
				// use modulo computation to make sure to not jump past the last frame
				if (loop)
				{
					if (activeAnimationFrame >= animationData[0]->cyclesData[activeAnimation].numberOfFrames)
						activeAnimationFrame = activeAnimationFrame % animationData[0]->cyclesData[activeAnimation].numberOfFrames;
				}
				else
					if (activeAnimationFrame >= animationData[0]->cyclesData[activeAnimation].numberOfFrames)
						activeAnimationFrame = animationData[0]->cyclesData[activeAnimation].numberOfFrames - 1;
			}
			else
			{
				// use modulo computation to make sure to not jump past the last frame
				if (loop)
				{
					if (activeAnimationFrame >= animationData[activeAnimation]->cyclesData[0].numberOfFrames)
						activeAnimationFrame = activeAnimationFrame % animationData[activeAnimation]->cyclesData[0].numberOfFrames;
				}
				else
					if (activeAnimationFrame >= animationData[activeAnimation]->cyclesData[0].numberOfFrames)
						activeAnimationFrame = animationData[activeAnimation]->cyclesData[0].numberOfFrames - 1;
			}
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