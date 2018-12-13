// INCLUDES /////////////////////////////////////////////////////////////////////////////

// C++ includes
#include "sstream"

// bell0bytes core
#include "app.h"
#include "playState.h"

// bell0bytes UI
#include "buttons.h"
#include "optionsMenuState.h"
#include "mainMenuState.h"
#include "keyMapMenuState.h"

// bell0bytes input
#include "gameCommands.h"
#include "inputComponent.h"
#include "inputHandler.h"

// bell0bytes graphics
#include "graphicsComponent.h"
#include "graphicsComponent3D.h"
#include "sprites.h"
#include "graphicsComponentWrite.h"

// bell0bytes file system
#include "fileSystemComponent.h"

// bell0bytes audio
#include "audioComponent.h"


// CLASS METHODS ////////////////////////////////////////////////////////////////////////
namespace UI
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructor and Destructor ////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	OptionsMenuState::OptionsMenuState(core::DirectXApp& app, const std::wstring& name) : GameState(app, name), currentlySelectedButton(0), wasInFullscreen(dxApp.getGraphicsComponent().getFullscreenState()), fullscreen(dxApp.getGraphicsComponent().getFullscreenState()), supportedModes(NULL), nSupportedModes(0), currentModeIndex(0)
	{
		dxApp.getGraphicsComponent().getFullscreenState();
	}

	OptionsMenuState::~OptionsMenuState()
	{ }

	OptionsMenuState& OptionsMenuState::createInstance(core::DirectXApp& app, const std::wstring& stateName)
	{
		static OptionsMenuState instance(app, stateName);
		return instance;
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Initialization //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::initialize()
	{
		// handle errors
		util::Expected<void> result;

		// hide the standard cursor
		ShowCursor(false);

		// position mouse at the center of the screen
		if (!SetCursorPos(dxApp.getGraphicsComponent().getCurrentWidth() / 2, dxApp.getGraphicsComponent().getCurrentHeight() / 2))
			return std::runtime_error("Critical error: Unable to set cursor position!");

		// allow only mouse input
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = false;

		// get fullscreen state
		fullscreen = dxApp.getGraphicsComponent().getFullscreenState();

		// get supported modes
		nSupportedModes = dxApp.getGraphicsComponent().get3DComponent().getNumberOfSupportedModes();
		supportedModes = dxApp.getGraphicsComponent().get3DComponent().getSupportedModes();
		currentModeIndex = dxApp.getGraphicsComponent().get3DComponent().getCurrentModeIndex();
		
		if (firstCreation)
		{
			// create text formats
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Lucida Handwriting", 128.0f, DWRITE_TEXT_ALIGNMENT_CENTER, titleFormat);
			if (!result.isValid())
				return result;

			result = dxApp.getGraphicsComponent().getWriteComponent().createTextFormat(L"Segoe Script", 48.0f, textFormat);
			if (!result.isValid())
				return result;

			// create text layouts
			std::wstring title = L"Game Options";
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWString(title, titleFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 200, titleLayout);
			if (!result.isValid())
				return result;

			std::wostringstream fullscreenText;
			fullscreenText << L"fullscreen\t\t\t\t\t" << fullscreen;
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(fullscreenText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, fullscreenLayout);
			if (!result.isValid())
				return result;

			std::wostringstream text;
			text << L"resolution\t\t\t\t\t" << dxApp.getGraphicsComponent().getCurrentWidth() << " x " << dxApp.getGraphicsComponent().getCurrentHeight() << " @ " << supportedModes[currentModeIndex].RefreshRate.Numerator / supportedModes[currentModeIndex].RefreshRate.Denominator << " Hz";
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(text, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, resolutionLayout);
			if (!result.isValid())
				return result;

			std::wostringstream soundEffectsVolumeText;
			float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Sound);
			soundEffectsVolumeText << L"effects volume\t\t" << volume;
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(soundEffectsVolumeText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, soundEffectsVolumeLayout);
			if (!result.isValid())
				return result;

			std::wostringstream musicVolumeText;
			volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Music);
			musicVolumeText << L"music volume\t\t" << volume;
			result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(musicVolumeText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, musicVolumeLayout);
			if (!result.isValid())
				return result;
		}

		// create button sound
		buttonClickSound = new audio::SoundEvent();
		result = dxApp.getAudioComponent().loadFile(dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Sounds, L"button.wav"), *buttonClickSound, audio::AudioTypes::Sound);
		if (!result.isValid())
			return result;

		// initialize buttons
		currentlySelectedButton = -1;
		if (!initializeButtons().wasSuccessful())
			return std::runtime_error("Critical error: Unable to create title buttons!");
		
		firstCreation = false;
		isPaused = false;
	
		// return success
		return {};
	}

	util::Expected<void> OptionsMenuState::initializeButtons()
	{
		std::vector<graphics::AnimationCycleData> animationCycles;
		graphics::AnimationCycleData cycle;
		graphics::AnimationData* animations;

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Fullscreen Selection ////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////
		
		// cycle
		cycle.name = L"Fullscreen Refresh Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Fullscreen Refresh Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonRefresh.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClick = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			this->fullscreen = !this->fullscreen;
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Fullscreen Toggle", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClick, 4)); }
		catch (std::exception& e) { return e; };

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Screen Resolution Left Arrow ////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Left Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonLeft.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickScreenResolutionLeftArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			if(this->currentModeIndex > 0)
				currentModeIndex--;
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Screen Resolution Left", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickScreenResolutionLeftArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Screen Resolution Right Arrow ///////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Right Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonRight.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickScreenResolutionRightArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			if (this->currentModeIndex < this->nSupportedModes-1)
				currentModeIndex++;
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Screen Resolution Right", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickScreenResolutionRightArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Music Volume Left Arrow /////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Left Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonLeft.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickMusicVolumeLeftArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// get volume
			float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Music);
			if (volume > 0)
				volume -= 0.1f;
			if (volume < 0.09f)
				volume = 0.0f;
			
			// set volume
			dxApp.getAudioComponent().setVolume(audio::AudioTypes::Music, volume);

			return {};
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Music Volume Left", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickMusicVolumeLeftArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////// Music Volume Right Arrow ////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Right Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonRight.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickMusicVolumeRightArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// get volume
			float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Music);
			if (volume < XAUDIO2_MAX_VOLUME_LEVEL)
				volume += 0.1f;
			if (volume > 9.91f)
				volume = 10;

			// set volume
			dxApp.getAudioComponent().setVolume(audio::AudioTypes::Music, volume);

			return {};
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Music Volume Right", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickMusicVolumeRightArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////// Sound Effects Volume Left Arrow //////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Left Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Left Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonLeft.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickSoundEffectsVolumeLeftArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// get volume
			float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Sound);
			if (volume > 0)
				volume -= 0.1f;
			if (volume < 0.09f)
				volume = 0.0f;

			// set volume
			dxApp.getAudioComponent().setVolume(audio::AudioTypes::Sound, volume);

			return {};
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Sound Effects Volume Left", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickSoundEffectsVolumeLeftArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////// Sound Effects Volume Right Arrow /////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Right Arrow Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Right Arrow Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonRight.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickSoundEffectsVolumeRightArrow = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// get volume
			float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Sound);
			if (volume < XAUDIO2_MAX_VOLUME_LEVEL)
				volume += 0.1f;
			if (volume > 9.91f)
				volume = 10;

			// set volume
			dxApp.getAudioComponent().setVolume(audio::AudioTypes::Sound, volume);

			return {};
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Sound Effects Volume Right", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickSoundEffectsVolumeRightArrow, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Gamepad Button ///////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Gamepad Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Gamepad Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 64;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonGamepad.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickGamepad = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// open key map menu
			if (!dxApp.pushGameState(&UI::KeyMapMenuState::createInstance(dxApp, L"Key Map Menu")).wasSuccessful())
				return std::runtime_error("Critical error: Unable to push key map menu to the state stack!");
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Gamepad", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickGamepad, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Button Save //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Save Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Save Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonSave.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickSave = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			// write options to lua file
			float soundEffectsVolume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Sound);
			float musicVolume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Music);
			dxApp.getFileSystemComponent().saveConfiguration(supportedModes[currentModeIndex].Width, supportedModes[currentModeIndex].Height, currentModeIndex, fullscreen, dxApp.getInputComponent().getInputHandler().activeJoystick, dxApp.getInputComponent().getInputHandler().activeGamepad, musicVolume, soundEffectsVolume);

			// activate desired screen resolution and fullscreen mode
			if (currentModeIndex != dxApp.getGraphicsComponent().get3DComponent().getCurrentModeIndex())
				dxApp.getGraphicsComponent().changeResolution(currentModeIndex);

			if (fullscreen != wasInFullscreen)
			{
				wasInFullscreen = !wasInFullscreen;
				dxApp.getGraphicsComponent().toggleFullscreen();
			}

			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Save", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickSave, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);

		/////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////// Back Button //////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////

		// cycle
		cycle.name = L"Back Normal";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Hover";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Click";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		cycle.name = L"Back Locked";
		cycle.startFrame = 0;
		cycle.numberOfFrames = 1;
		cycle.width = 65;
		cycle.height = 64;
		cycle.paddingWidth = 0;
		cycle.paddingHeight = 0;
		cycle.borderPaddingHeight = cycle.borderPaddingWidth = 0;
		cycle.rotationCenterX = cycle.rotationCenterY = 0.5f;
		animationCycles.push_back(cycle);

		// create play button animations
		try { animations = new graphics::AnimationData(d2d, dxApp.getFileSystemComponent().openFile(fileSystem::DataFolders::Buttons, L"buttonBack.png").c_str(), animationCycles); }
		catch (std::runtime_error& e) { return e; }

		auto onClickBack = [this]() -> util::Expected<void>
		{
			dxApp.getAudioComponent().playSoundEvent(*buttonClickSound);
			Sleep(120);

			this->isPaused = true;
			if (!dxApp.changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
				return std::runtime_error("Critical error: Unable to change game state to main menu!");
			return { };
		};

		// add button to the list
		try { menuButtons.push_back(new AnimatedButton(L"Back", new graphics::AnimatedSprite(d2d, animations, 0, 24), onClickBack, 4)); }
		catch (std::exception& e) { return e; }

		// clear animation data
		animationCycles.clear();
		std::vector<graphics::AnimationCycleData>(animationCycles).swap(animationCycles);
		
		// set to unpaused
		this->isPaused = false;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Pause and Resume //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::pause()
	{
		isPaused = true;

		// return success
		return {};
	}

	util::Expected<void> OptionsMenuState::resume()
	{
		// allow mouse and keyboard input
		dxApp.getInputComponent().getInputHandler().activeMouse = true;
		dxApp.getInputComponent().getInputHandler().activeKeyboard = false;

		isPaused = false;
		currentlySelectedButton = -1;

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// User Input //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::onMessage(const core::Depesche& depesche)
	{
		input::InputHandler* ih = (input::InputHandler*)depesche.sender;

		if (!isPaused)
			if (!ih->isListening())
				return handleInput(ih->activeKeyMap);

		// return success
		return { };
	}

	util::Expected<void> OptionsMenuState::handleInput(std::unordered_map<input::GameCommands, input::GameCommand&>& activeKeyMap)
	{
		// act on user input
		for (auto x : activeKeyMap)
		{
			switch (x.first)
			{
			case input::Select:
				// activate currently selected button
				if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
					return menuButtons[currentlySelectedButton]->click();
				else
					break;

			case input::ShowFPS:
				dxApp.toggleFPS();
				break;

			case input::Back:
					isPaused = true;
					if (!dxApp.changeGameState(&UI::MainMenuState::createInstance(dxApp, L"Main Menu")).wasSuccessful())
						return std::runtime_error("Critical error: Unable to change game state to main menu!");
					break;
			}
		}

		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////// Update /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::update(const double deltaTime)
	{
		// handle errors
		util::Expected<void> result;

		if (isPaused)
			return {};

		// capture mouse
		if (dxApp.getInputComponent().getInputHandler().activeMouse)
		{
			// get mouse position
			long mouseX = dxApp.getInputComponent().getInputHandler().getMouseX();
			long mouseY = dxApp.getInputComponent().getInputHandler().getMouseY();

			// check if mouse position is inside button rectangle
			bool buttonSelected = false;
			int i = 0;
			for (auto button : menuButtons)
			{
				D2D1_RECT_F rect = button->getRectangle();
				if (mouseX > rect.left && mouseX < rect.right && mouseY > rect.top && mouseY < rect.bottom)
				{
					if (currentlySelectedButton != i)
					{
						// deselect current button
						if (currentlySelectedButton >= 0 && currentlySelectedButton < menuButtons.size())
							menuButtons[currentlySelectedButton]->deselect();

						// select button
						button->select();
						currentlySelectedButton = i;
						buttonSelected = true;
					}
					else
						button->select();

					buttonSelected = true;
				}
				else
					button->deselect();
				i++;
			}
			if (!buttonSelected)
				currentlySelectedButton = -1;
		}

		// update fullscreen text
		std::wostringstream fullscreenText;
		fullscreenText << L"fullscreen     " << std::boolalpha << fullscreen;
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(fullscreenText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, fullscreenLayout);
		if (!result.isValid())
			return result;

		// update screen resolution text
		std::wostringstream screenResolutionText;
		screenResolutionText << L"resolution\t   " << supportedModes[currentModeIndex].Width << " x " << supportedModes[currentModeIndex].Height << " @ " << supportedModes[currentModeIndex].RefreshRate.Numerator / supportedModes[currentModeIndex].RefreshRate.Denominator << " Hz";
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(screenResolutionText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, resolutionLayout);
		if (!result.isValid())
			return result;

		// update volume texts
		std::wostringstream soundEffectsVolumeText;
		soundEffectsVolumeText.precision(2);
		float volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Sound);
		soundEffectsVolumeText << L"effects volume\t\t" << volume;
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(soundEffectsVolumeText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, soundEffectsVolumeLayout);
		if (!result.isValid())
			return result;

		if(volume <= 0.0f)
			menuButtons[5]->lock();
		if (volume >= 10)
			menuButtons[6]->lock();
		
		std::wostringstream musicVolumeText;
		musicVolumeText.precision(2);
		volume = dxApp.getAudioComponent().getVolume(audio::AudioTypes::Music);
		musicVolumeText << L"music volume\t\t" << volume;
		result = dxApp.getGraphicsComponent().getWriteComponent().createTextLayoutFromWStringStream(musicVolumeText, textFormat.Get(), (float)dxApp.getGraphicsComponent().getCurrentWidth(), 100, musicVolumeLayout);
		if (!result.isValid())
			return result;

		if (volume <= 0.0f)
			menuButtons[3]->lock();
		if (volume >= 10)
			menuButtons[4]->lock();


		for (auto button : menuButtons)
			button->update(deltaTime);

		if (currentModeIndex == 0)
		{
			menuButtons[1]->lock();
			if (currentlySelectedButton == 1)
				currentlySelectedButton = -1;
		}

		if (currentModeIndex == nSupportedModes - 1)
		{
			menuButtons[2]->lock();
			if (currentlySelectedButton == 2)
				currentlySelectedButton = -1;
		}

		// lock resolution buttons for now as we do not have resizable graphics yet
		menuButtons[1]->lock();
		menuButtons[2]->lock();

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Render //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::render(const double /*farSeer*/)
	{
		// error handling
		util::Expected<void> result;

		if (!isPaused)
		{
			dxApp.getGraphicsComponent().getWriteComponent().printText(0, 50, titleLayout.Get());// , 0, -300);

			dxApp.getGraphicsComponent().getWriteComponent().printCenteredText(fullscreenLayout.Get(), -50, -130);
			menuButtons[0]->drawCentered(1, 0, -180);

			dxApp.getGraphicsComponent().getWriteComponent().printCenteredText(resolutionLayout.Get(), -95, -38);
			menuButtons[1]->drawCentered(1, -50, -90);
			menuButtons[2]->drawCentered(1, 50, -90);

			dxApp.getGraphicsComponent().getWriteComponent().printCenteredText(musicVolumeLayout.Get(), -275, 60);
			menuButtons[3]->drawCentered(0.75f, -50, 10);
			menuButtons[4]->drawCentered(0.75f, 50, 10);

			dxApp.getGraphicsComponent().getWriteComponent().printCenteredText(soundEffectsVolumeLayout.Get(), -275, 125);
			menuButtons[5]->drawCentered(0.75f, -50, 70);
			menuButtons[6]->drawCentered(0.75f, 50, 70);
			
			menuButtons[7]->drawCentered(2, 0, 200);
			menuButtons[8]->drawCentered(2, -300, 300);
			menuButtons[9]->drawCentered(2, 300, 300);

			// print FPS information
			dxApp.getGraphicsComponent().getWriteComponent().printFPS();
		}
		menuButtons[7]->drawCentered(2, 0, 200);

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Shutdown ////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> OptionsMenuState::shutdown()
	{
		// stop button sound
		if (buttonClickSound)
			dxApp.getAudioComponent().stopSoundEvent(*buttonClickSound);

		ShowCursor(false);
		this->isPaused = true;

		// delete buttons
		for (auto button : menuButtons)
			delete button;
		menuButtons.clear();

		// delete sound
		if (buttonClickSound)
			delete buttonClickSound;

		// return success
		return {};
	}
}