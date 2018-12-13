#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		14/06/2018 - Lenningen - Luxembourg
*
* Desc:		main state of the running game
*
* Hist:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <iostream>
#include <array>

// Microsoft
#include <wrl.h>

// direct write
#include <dwrite_3.h>

// bell0bytes core
#include "states.h"
#include "depesche.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////
namespace UI
{
	class HeadsUpDisplayState;
}

namespace graphics
{
	class Sprite;
	class AnimatedSprite;
	class AnimatedSprite;
}

namespace core
{
	class DirectXApp;
	class DepescheSender;
	class DepescheDestination;
}

namespace audio
{
	struct SoundEvent;
};

namespace game
{
	class DirectXApp;

	enum WalkDirection {Left, Right};

	class Entity
	{
	protected:
		std::vector<graphics::AnimatedSprite*> sprites;		// the sprites of the entity
		float x, y;											// current position of the entity
		float health;										// the health of the entity
		unsigned int walkDirection = 0;						// the direction the entity is walking in
		bool dead = false;									// true iff the entity is dead
		bool idle = true;									// true iff the entity is idle
		bool running = false;								// true iff the entity is running
		const float velocity;								// the velocity of the entity
		WalkDirection direction;							// the direction the entity is moving to
		bool justKilled = false;							// true iff the entity died during this frame

	public:
		Entity(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity);
		Entity(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity);
		virtual ~Entity(); 

		// add an animation to the game entity
		void addAnimation(graphics::AnimatedSprite* const animation);
		
		// update the game entity
		virtual util::Expected<void> update(core::DirectXApp& dxApp, const double deltaTime) = 0;

		friend class PlayState;
	};

	class NPC : public Entity, public core::DepescheDestination, public core::DepescheSender
	{
	private:
		bool badHealth = false;			// true iff the cat has not many lives left
		bool vibrate = false;			// true iff the dog is close
		float vibrationSpeed = 0.0f;	// vibrate gamepad if cats are attacked by the dog

	public:
		NPC(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity);
		NPC(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity);

		// update the NPCs
		util::Expected<void> update(core::DirectXApp& dxApp, const double deltaTime) override;
		util::Expected<void> update(core::DirectXApp& dxApp, const double deltaTime, const audio::SoundEvent& soundEvent);

		// interaction with other entities
		util::Expected<void> onMessage(const core::Depesche&) override;
	};

	class Player : public Entity, public core::DepescheSender
	{
	public:
		// variables to move the game entity
		float moveX, moveY;
		bool chaseCats = true;

		// constructors
		Player(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity);
		Player(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity);

		// update the game entity
		util::Expected<void> update(core::DirectXApp& dxApp, const double deltaTime) override;

		// getters for the position of the game entity
		const float getX() const { return x; };
		const float getY() const { return y; };

		// stop moving after all cats are defeated
		void stop();
	};

	class PlayState : public core::GameState
	{
	private:
		// the entities
		std::vector<NPC*> cats;
		Player* dog;
		unsigned int nAliveCats = 5;
		bool gameOver = false;

		// cat meow
		audio::SoundEvent* catMeow;
		audio::SoundEvent* dogBark;

		// the HUD
		UI::HeadsUpDisplayState* hud;

		// the constructor is private to ensure that this will be a singleton
		PlayState(core::DirectXApp& app, const std::wstring& name);

		// initialize game objects
		util::Expected<void> initializeGameEntities();

	public:
		virtual ~PlayState();

		// singleton: get instance
		static PlayState& createInstance(core::DirectXApp& app, const std::wstring& name);

		// initialization
		virtual util::Expected<void> initialize() override;
		virtual util::Expected<void> shutdown() override;

		// pause and resume
		virtual util::Expected<void> pause() override;
		virtual util::Expected<void> resume() override;

		// user input
		virtual util::Expected<void> handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap) override;
		virtual util::Expected<void> update(const double deltaTime) override;

		// render the scene
		virtual util::Expected<void> render(const double farSeer) override;

		// handle message
		virtual util::Expected<void> onMessage(const core::Depesche&) override;
	};
}