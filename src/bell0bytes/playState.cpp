// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes: streams
#include <sstream>

// class header
#include "playState.h"

// bell0bytes core
#include "app.h"
#include "depesche.h"

// bell0bytes file system
#include "fileSystemComponent.h"
#include "folders.h"

// bell0bytes UI
#include "gameMenuState.h"
#include "headsUpDisplayState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputComponent.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponent2D.h"
#include "d2d.h"
#include "graphicsComponentWrite.h"
#include "sprites.h"

// bell0bytes audio
#include "audioComponent.h"

// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace game
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	PlayState::PlayState(core::DirectXApp& app, const std::wstring& name) : GameState(app, name)
	{ }
	PlayState::~PlayState()
	{ }
	PlayState& PlayState::createInstance(core::DirectXApp& app, const std::wstring& stateName)
	{
		static PlayState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::initialize()
	{
		// catch errors
		util::Expected<void> result;
		
		// allow only keyboard input
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;
		dxApp.getInputComponent().getInputHandler().activeMouse = false;

		// notify the main application class that the game is running
		isPaused = false;

		// create the cat meow sound
		catMeow = new audio::SoundEvent();
		result = dxApp.getAudioComponent().loadFile(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Sounds, L"catMeow.wav"), *catMeow, audio::AudioTypes::Sound);
		if (!result.isValid())
			return result;

		// create the dog bark sound
		dogBark = new audio::SoundEvent();
		result = dxApp.getAudioComponent().loadFile(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Sounds, L"dogBark.wav"), *dogBark, audio::AudioTypes::Sound);
		if (!result.isValid())
			return result;

		// initialize game entities
		initializeGameEntities();

		// initialize the game overlay
		hud = &UI::HeadsUpDisplayState::createInstance(dxApp, L"HUD");
		dxApp.overlayGameState(hud);

		// send depesche to play barking sound
		core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::PlaySoundEvent, dogBark);
		dxApp.addMessage(depesche);

		firstCreation = false;
		gameOver = false;

		// return success
		return { };
	}

	util::Expected<void> PlayState::initializeGameEntities()
	{
		// create a few cats
		for (unsigned int i = 0; i < 5; i++)
		{
			std::vector<graphics::AnimationData*> catAnimations;
			graphics::AnimationCycleData cycle;

			/////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////// Walk ///////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////

			// cycle
			cycle.name = L"Cat Walk";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 10;
			cycle.width = 287;
			cycle.height = 500;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

			// create walk animation
			try { catAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"catWalk.png").c_str(), cycle)); }
			catch (std::runtime_error& e) { return e; }

			/////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////// Dead ///////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////

			// cycle
			cycle.name = L"Cat Dead";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 10;
			cycle.width = 600;
			cycle.height = 500;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

			// create dead animation
			try { catAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"catDead.png").c_str(), cycle)); }
			catch (std::runtime_error& e) { return e; }

			/////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////// Hurt ///////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////

			// cycle
			cycle.name = L"Cat Hurt";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 10;
			cycle.width = 600;
			cycle.height = 500;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

			// create dead animation
			try { catAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"catHurt.png").c_str(), cycle)); }
			catch (std::runtime_error& e) { return e; }

			// create cat
			try 
			{ 
				float x = (float) (rand() % 1800);
				float y = (float) (rand() % 1000);
				cats.push_back(new NPC(new graphics::AnimatedSprite(d2d, catAnimations, 0, 11, x, y), x, y, 6, 15));
			}
			catch (std::runtime_error& e) { return e; }

			// set the walk direction to "right"
			cats[i]->walkDirection = WalkDirection::Right;
		}

		// create health bars
		for (auto cat : cats)
		{
			std::vector<graphics::AnimationCycleData> animationCycles;
			graphics::AnimationCycleData cycle;
			graphics::AnimationData* animations;

			/////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////// Healthbar 1 /////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////

			// cycle
			cycle.name = L"Healthbar 1";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);

			cycle.name = L"Healthbar 2";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);
			
			cycle.name = L"Healthbar 3";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);

			cycle.name = L"Healthbar 4";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);

			cycle.name = L"Healthbar 5";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);

			cycle.name = L"Healthbar 6";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);

			cycle.name = L"Healthbar 7";
			cycle.startFrame = 0;
			cycle.numberOfFrames = 1;
			cycle.width = 28;
			cycle.height = 8;
			cycle.paddingWidth = 0;
			cycle.paddingHeight = 0;
			cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
			cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
			animationCycles.push_back(cycle);
			
			// create animations
			try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Bars, L"healthBar.png").c_str(), animationCycles); }
			catch (std::runtime_error& e) { return e; }

			// add bar to list
			try { cat->addAnimation(new graphics::AnimatedSprite(d2d, animations, 0, 24, cat->x, cat->y - 125)); }
			catch (std::exception& e) { return e; };

			// clear animation data
			animationCycles.clear();
			std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);
		}

		// create the dog
		std::vector<graphics::AnimationData*> dogAnimations;
		graphics::AnimationCycleData cycle;

		/////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////// Walk ///////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Dog Walk";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 10;
		cycle.width = 292;
		cycle.height = 500;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

		// create walk animation
		try { dogAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"dogWalk.png").c_str(), cycle)); }
		catch (std::runtime_error& e) { return e; }

		/////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////// Run ////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Dog Run";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 8;
		cycle.width = 293;
		cycle.height = 500;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

		// create walk animation
		try { dogAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"dogRun.png").c_str(), cycle)); }
		catch (std::runtime_error& e) { return e; }

		/////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////// Idle ///////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Dog Idle";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 10;
		cycle.width = 292;
		cycle.height = 500;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;

		// create walk animation
		try { dogAnimations.push_back(new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Entities, L"dogIdle.png").c_str(), cycle)); }
		catch (std::runtime_error& e) { return e; }
		
		// create the dog
		float x = dxApp.getGraphicsComponent().getCurrentWidth() / 2.0f;
		float y = dxApp.getGraphicsComponent().getCurrentHeight() / 2.0f;
		try { dog = new Player(new graphics::AnimatedSprite(d2d, dogAnimations, 0, 32, x, y), x, y, 100, 25); }
		catch (std::runtime_error& e) { return e; }

		// set walk direction to "right"
		dog->walkDirection = WalkDirection::Right;

		return { };
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::pause()
	{
		isPaused = true;

		// return success
		return { };
	}
	util::Expected<void> PlayState::resume()
	{
		// allow only keyboard input
		dxApp.getInputComponent().getInputHandler().activeKeyboard = true;
		dxApp.getInputComponent().getInputHandler().activeMouse = false;

		isPaused = false;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::GameCommands::Select:
				break;
			default:
				break;
			}
		}

		return { };	
	}

	util::Expected<void> PlayState::onMessage(const core::Depesche& depesche)
	{
		if (depesche.type == core::DepescheTypes::ActiveKeyMap)
		{
			input::InputHandler* ih = (input::InputHandler*)depesche.sender;

			if (!isPaused)
				if (!ih->isListening())
					return handleInput(ih->activeKeyMap);
		}
		
		if (depesche.type == core::DepescheTypes::Gamepad)
		{
			input::InputHandler* ih = (input::InputHandler*)depesche.sender;

			dog->moveX = ih->getLX();
			dog->moveY = ih->getLY();
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::update(const double deltaTime)
	{
		if (isPaused)
			return { };

		util::Expected<void> result;

		// update the player entity
		result = dog->update(dxApp, deltaTime);
		if (!result.isValid())
			return result;
		
		// update the NPCs
		for (auto cat : cats)
		{
			// notify the NPCs about the dogs position
			core::Depesche depesche(*dog, *cat, core::DepescheTypes::Damage, nullptr);
			dxApp.addMessage(depesche);
			
			result = cat->update(dxApp, deltaTime, *catMeow);
			if (!result.isValid())
				return result;
		}

		// send message to the HUD about the state of the game
		nAliveCats = 0;
		for (auto cat : cats)
			if (!cat->dead)
				nAliveCats++;
		if (nAliveCats == 0)
			if (!gameOver)
			{
				dog->stop();
				
				// send depesche to play barking sound
				core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::PlaySoundEvent, dogBark);
				dxApp.addMessage(depesche);
				
				gameOver = true;
				dxApp.getInputComponent().getInputHandler().vibrateGamepad(0.0f, 0.0f);
			}

		core::Depesche depesche(*this, *hud, core::DepescheTypes::Damage, &nAliveCats);
		dxApp.addMessage(depesche);

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::render(const double /*farSeer*/)
	{
		if (!isPaused)
		{
			// draw the cats
			for (auto cat : cats)
			{
				if (cat->walkDirection == WalkDirection::Left)
				{
					dxApp.getGraphicsComponent().get2DComponent().reflectY(cat->x, cat->y);
					cat->sprites[0]->draw(0.5f);	// the actual cat
					if(!cat->dead)
						cat->sprites[1]->draw(2.0f);	// the health bar
					dxApp.getGraphicsComponent().get2DComponent().resetTransformation();
				}
				else
				{
					cat->sprites[0]->draw(0.5f);		// the actual cat
					if(!cat->dead)
						cat->sprites[1]->draw(2.0f);	// the health bar
				}
			}

			// draw the dog
			if (dog->walkDirection == WalkDirection::Left)
			{
				dxApp.getGraphicsComponent().get2DComponent().reflectY(dog->x, dog->y);
				dog->sprites[0]->draw(0.5f);
				dxApp.getGraphicsComponent().get2DComponent().resetTransformation();
			}
			else
				dog->sprites[0]->draw(0.5f);
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> PlayState::shutdown()
	{
		isPaused = true;

		// stop barking and meowing
		if (catMeow)
			dxApp.getAudioComponent().stopSoundEvent(*catMeow);
		if (dogBark)
			dxApp.getAudioComponent().stopSoundEvent(*dogBark);
		
		// delete the cats
		for (auto cat : cats)
			delete cat;
		cats.clear();

		// delete the dog
		delete dog;

		// delete sounds
		if (catMeow)
			delete catMeow;
		if (dogBark)
			delete dogBark;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Entities ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Entity::Entity(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity) : x(x), y(y), health(health), velocity(velocity)
	{
		sprites.push_back(sprite);
	};

	Entity::Entity(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity) : sprites(sprites), x(x), y(y), health(health), velocity(velocity)
	{

	};

	NPC::NPC(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity) : Entity(sprite, x, y, health, velocity)
	{ };

	NPC::NPC(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity) : Entity(sprites, x, y, health, velocity)
	{ };

	Player::Player(graphics::AnimatedSprite* const sprite, const float x, const float y, const float health, const float velocity) : Entity(sprite, x, y, health, velocity)
	{ };

	Player::Player(std::vector<graphics::AnimatedSprite*>& sprites, const float x, const float y, const float health, const float velocity) : Entity(sprites, x, y, health, velocity)
	{ };
	
	Entity::~Entity()
	{
		for (auto sprite : sprites)
			delete sprite;
		sprites.clear();
	}

	void Entity::addAnimation(graphics::AnimatedSprite* const animData)
	{
		sprites.push_back(animData);
	}

	util::Expected<void> Player::update(core::DirectXApp& dxApp, const double deltaTime)
	{
		if (chaseCats)
		{
			// set the direction
			if (moveX >= 0.0f)
				walkDirection = WalkDirection::Right;
			else
				walkDirection = WalkDirection::Left;

			// change animation based on speed
			if (moveX == 0 && moveY == 0)
			{
				if (!idle)
				{
					idle = true;
					sprites[0]->changeAnimation(2);
				}
			}
			else
			{
				// change animation if the dog is running
				if (std::fabs(moveX) > 0.75f || std::fabs(moveY) > 0.75)
				{
					if (!running)
					{
						running = true;
						sprites[0]->changeAnimation(1);
					}
				}
				else
					if (running || idle)
					{
						running = false;
						idle = false;
						sprites[0]->changeAnimation(0);
					}
			}

			// update position
			x += moveX * velocity;
			y -= moveY * velocity;

			// check boundaries
			if (x < 200) x = 200; if (y < 200) y = 200; if (x > 1850) x = 1850; if (y > 950) y = 950;

			// update sprite
			sprites[0]->setPosition(x, y);
			sprites[0]->updateAnimation(deltaTime);
		}
		else
		{
			x = dxApp.getGraphicsComponent().getCurrentWidth() / 2.0f;
			y = dxApp.getGraphicsComponent().getCurrentHeight() / 2.0f - 150.0f;
			sprites[0]->setPosition(x, y);
			sprites[0]->updateAnimation(deltaTime);
		}
				
		return { };
	}

	util::Expected<void> NPC::update(core::DirectXApp& dxApp, const double deltaTime)
	{
		// update NPCs
		if (dead)
		{
			sprites[0]->updateAnimation(deltaTime, false);
			return { };
		}

		// check life
		if (health == 0)
		{
			// the cat was just killed
			dead = true;
			justKilled = true;
			
			sprites[0]->changeAnimation(1);
			x += -25 + rand() % 50;
			y += -25 + rand() % 50;
		}

		// update the cat animations
		for (auto sprite : sprites)
			sprite->updateAnimation(deltaTime);

		// vibrate
		if (vibrate)
		{
			if (health >= 3)
			{
				vibrationSpeed = 0.5f;
				core::Depesche depesche(*this, dxApp.getInputComponent().getInputHandler(), core::DepescheTypes::Gamepad, &vibrationSpeed);
				dxApp.addMessage(depesche);
			}
			else if (health >= 2)
			{
				vibrationSpeed = 0.75f;
				core::Depesche depesche(*this, dxApp.getInputComponent().getInputHandler(), core::DepescheTypes::Gamepad, &vibrationSpeed);
				dxApp.addMessage(depesche);
			}
			else if (health > 0)
			{
				vibrationSpeed = 1.0f;
				core::Depesche depesche(*this, dxApp.getInputComponent().getInputHandler(), core::DepescheTypes::Gamepad, &vibrationSpeed);
				dxApp.addMessage(depesche);
			}
		}
		else
		{
			vibrationSpeed = 0.0f;
			core::Depesche depesche(*this, dxApp.getInputComponent().getInputHandler(), core::DepescheTypes::Gamepad, &vibrationSpeed);
			dxApp.addMessage(depesche);
		}

		return { };
	}

	util::Expected<void> NPC::update(core::DirectXApp& dxApp, const double deltaTime, const audio::SoundEvent& soundEvent)
	{
		util::Expected<void> result = update(dxApp, deltaTime);
		if (!result.isValid())
			return result;

		if (justKilled)
		{
			// send depesche to play meow sound
			core::Depesche depesche(*this, dxApp.getAudioComponent(), core::DepescheTypes::PlaySoundEvent, (void*)&soundEvent);
			dxApp.addMessage(depesche);

			justKilled = false;
		}
		
		// return success
		return { };
	}

	util::Expected<void> NPC::onMessage(const core::Depesche& depesche)
	{
		if (dead)
			// do nothing
			return { };
		
		// reset vibration state
		vibrate = false;

		// get the dog
		Player* dog = (Player*)depesche.sender;

		// track old state
		float oldX = x;
		float oldY = y;
		
		// check for dog attacks
		if (dog->getX() > x - 50 && dog->getX() < x + 50 && dog->getY() > y - 50 && dog->getY() < y + 50)
			if (health > 0)
			{
				health--;
				sprites[1]->changeAnimation(6 - (unsigned int)health);

				// play hurt animation if the cat has lost some of her lives
				if (health < 3 && !badHealth)
				{
					badHealth = true;
					sprites[0]->changeAnimation(2);
				}

				vibrate = true;
			}

		// change speed based on health
		float currentVelocity = velocity;
		if (health < 3)
			currentVelocity = velocity * 0.6f;

		// move away from the dog
		if (dog->getX() < x)
		{
			x += currentVelocity;
			walkDirection = WalkDirection::Right;
		}
		else
		{
			x -= currentVelocity;
			walkDirection = WalkDirection::Left;
		}

		if (dog->getY() < y)
			y += currentVelocity;
		else
			y -= currentVelocity;

		if (x < 250)
			x = 250;
		if (y < 250)
			y = 250;
		if (x > 1800)
			x = 1800;
		if (y > 900)
			y = 900;

		// if against the wall: try to burst away by moving around randomly
		if (oldX == x && oldY == y)
		{
			if (x == 250)
				// move to the right
				x += rand() % 100;
			else if (x == 1800)
				// move to the left
				x -= rand() % 100;

			if (y == 250)
				// move down
				y += rand() % 50;
			else if (y == 900)
				// move up
				y -= rand() % 50;
		}

		// set the final position
		sprites[0]->setPosition(x, y);
		
		// set the health bar position
		sprites[1]->setPosition(x, y - 125);

		return { };
	}

	void Player::stop()
	{
		sprites[0]->changeAnimation(2);
		chaseCats = false;
	}

}