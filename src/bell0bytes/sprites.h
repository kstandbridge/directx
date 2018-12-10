#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		28/05/2018 - Lenningen - Luxembourg
*
* Desc:		Sprites!
*
* History:
*			- 29/05/18: AnimatedSprites added
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// C++ includes
#include <string.h>
#include <map>
#include <vector>

// Windows and COM
#include <wrl/client.h>

// DirectX includes
#include <d2d1_3.h>

// bell0bytes utilities
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace graphics
{
	// forward declarations
	class Direct2D;

	// layers are used in the painter's algorithm
	enum Layers { Background, Characters, UserInterface };

	// specify what to draw
	enum DrawCommands { All, onlyBackground, onlyCharacters, onlyUserInterface };

	class Sprite
	{
	protected:
		Direct2D* const d2d;								// pointer to the Direct2D class
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> bitmap;		// a bitmap of the actual image
		Layers layer;										// the layer the bitmap belongs to
		unsigned int drawOrder;								// the draw order of the bitmap; relative to the layer
		float x, y;											// position of the bitmap
		D2D1_SIZE_F size;									// the width and height of the sprite

	public:
		// constructors and destructors
		Sprite(Direct2D* const d2d, ID2D1Bitmap1* const bitmap, const float x = 0.0f, const float y = 0.0f, const Layers layer = Layers::Characters, const unsigned int drawOrder = 0);		// create a sprite from an existing bitmap
		Sprite(Direct2D* const d2d, LPCWSTR imageFile, const float x = 0.0f, const float y = 0.0f, const Layers layer = Layers::Characters, const unsigned int drawOrder = 0);	// loads an image from the disk and saves it as a sprite
		~Sprite();

		// drawing
		D2D1_RECT_F getCenteredRectangle(const float scaleFactor) const;	// returns a destination rectangle with the sprite at the center of the screen (scaled by scaleFactor)
		void draw(const D2D1_RECT_F* const destRect, const D2D1_RECT_F* const sourceRect, const float opacity = 1.0f, const D2D1_BITMAP_INTERPOLATION_MODE interPol = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR) const;		// draws the sprite at the given location, with the given opacity and interpolation mode	
		void drawCentered(const float scaleFactor = 1.0f, const float xOffset = 0.0f, const float yOffset = 0.0f, const float opacity = 1.0f, D2D1_BITMAP_INTERPOLATION_MODE interPol = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, const D2D1_RECT_F* const sourceRect = NULL) const;

		// getters and setters
		void setPosition(float posX, float posY);			// sets the position of the sprite to the location specified by posX and posY
		float getWidth() const { return size.width; };
		float getHeight() const { return size.height; };

		// friends
		friend class SpriteMap;
		friend class AnimatedSprite;
	};

	// structure to define animation cycles
	struct AnimationCycleData
	{
		LPCWSTR name;					// name of the animation
		unsigned int startFrame;		// the index of the first frame of an animation
		unsigned int numberOfFrames;	// the total numbers of frames in the animation
		float	width,					// the width of each frame
				height,					// the height of each frame
				rotationCenterX,		// rotation center x-coordinate
				rotationCenterY,		// rotation center y-coordinate
				paddingWidth,			// width of the padding
				paddingHeight,			// height of the padding
				borderPaddingWidth,		// width of the border padding
				borderPaddingHeight;	// height of the border padding
	};

	// class to store animations
	class AnimationData
	{
	private:
		Microsoft::WRL::ComPtr<ID2D1Bitmap1> spriteSheet;	// the image containing all the animations
		std::vector<AnimationCycleData> cyclesData;			// the cycle data for all the different cycles in this animation

	public:
		AnimationData(Direct2D* const d2d, LPCWSTR spriteSheetFile, const std::vector<AnimationCycleData>& frameData);
		~AnimationData();

		friend class AnimatedSprite;
	};

	class AnimatedSprite : public Sprite
	{
	private:
		AnimationData* animationData;					// the data and the spritesheet for this sprite
		unsigned int activeAnimation;					// the currently active animation
		unsigned int activeAnimationFrame;				// the currently active frame
		const float animationFPS = 24.0f;				// the animation's own FPS
		double frameTime;								// the time the current frame has been displayed

	public:
		// constructor and destructor
		AnimatedSprite(Direct2D* const d2d, AnimationData* const animData, const unsigned int activeAnimation = 0, const float animationFPS = 24, const float x = 0.0f, const float y = 0.0f, const Layers layer = Layers::Characters, const unsigned int drawOrder = 0);	// creates an animated sprite with corresponding animation data structure and sets the active animation as specified
		~AnimatedSprite();

		// drawing
		void draw() const;										// the draw functions computes the source and destination rectangle and then calls on the Sprite::draw method to actually draw the AnimatedSprite

		// update and change
		void updateAnimation(const double deltaTime);					// updates the currently active animation cycle based on the passed time
		void changeAnimation(const unsigned int cycleToActivate);		// activated the specified animation cycle
	};

	class SpriteMap
	{
	private:
		std::multimap<unsigned int, Sprite* const> backgroundMap;		// holds the background sprites
		std::multimap<unsigned int, Sprite* const> characterMap;		// holds the character sprites
		std::multimap<unsigned int, Sprite* const> userInterfaceMap;	// holds the user interface sprites

	public:
		// constructors
		SpriteMap();
		~SpriteMap();

		// populate the sprite map
		void addSprite(Sprite* const sprite);							// adds an existing sprite to its correct map
		util::Expected<void> addSprite(Direct2D* const d2d, LPCWSTR imageFile, const float x = 0.0f, const float y = 0.0f, const Layers layer = Layers::Characters, unsigned int drawOrder = 0);	// create a new sprite and adds it to sprite map

		// draw the sprites
		void draw(D2D1_RECT_F* const destRect, D2D1_RECT_F* const sourceRect, const DrawCommands drawCommand = DrawCommands::All, const float opacity = 1.0f, const D2D1_BITMAP_INTERPOLATION_MODE interPol = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR) const;	// draw sprites based on layers and draw orders
	};
}