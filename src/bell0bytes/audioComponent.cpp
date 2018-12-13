// header file
#include "audioComponent.h"

// bell0bytes core
#include "app.h"

// bell0bytes filesystem
#include "fileSystemComponent.h"

// bell0bytes audio
#include "XAudio2.h"

// bell0bytes util
#include "serviceLocator.h"
#include "stringConverter.h"

// Lua and Sol

// warning supression no longer needed with C++-17
//#pragma warning( push )
//#pragma warning( disable : 4127)	// disable constant if expr warning
//#pragma warning( disable : 4702)	// disable unreachable code warning
#include <sol.hpp>
//#pragma warning( pop ) 
#pragma comment(lib, "liblua53.a")

namespace audio
{
	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////// Constructors and Destructor ///////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	AudioComponent::AudioComponent(const core::DirectXApp& dxApp) : dxApp(dxApp), streamingThread(nullptr)
	{
		try { engine = new AudioEngine(); }
		catch (std::runtime_error& e) { throw e; }

		// create the submix voices
		HRESULT hr = S_OK;
		
		hr = engine->dev->CreateSubmixVoice(&soundsSubmix, 1, 44100, 0, 0, 0, 0);
		if (FAILED(hr))
			throw std::runtime_error("Critical error: Unable to create voice submix!");
		hr = engine->dev->CreateSubmixVoice(&musicSubmix, 1, 44100, 0, 0, 0, 0);
		if (FAILED(hr))
			throw std::runtime_error("Critical error: Unable to create voice submix!");
		
		sendSounds = { 0, soundsSubmix };
		soundsSendList = { 1, &sendSounds };
		sendMusic = { 0, musicSubmix };
		musicSendList = { 1, &sendMusic };

		// set volume level
		loadVolume();
		soundsSubmix->SetVolume(soundEffectsVolume);
		musicSubmix->SetVolume(musicVolume);
	}

	AudioComponent::~AudioComponent()
	{
		endStream();

		ZeroMemory(&soundsSendList, sizeof(soundsSendList));
		ZeroMemory(&musicSendList, sizeof(musicSendList));

		ZeroMemory(&sendSounds, sizeof(sendSounds));
		ZeroMemory(&sendMusic, sizeof(sendMusic));
		
		// delete submixes
		soundsSubmix->DestroyVoice();
		musicSubmix->DestroyVoice();
		
		engine->dev->StopEngine();

		if (engine)
			delete engine;

		// delete thread
		if (streamingThread)
			delete streamingThread;
	}

	SoundEvent::SoundEvent() : sourceVoice(nullptr)
	{
		audioData.clear();
		ZeroMemory(&audioBuffer, sizeof(XAUDIO2_BUFFER));
		ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
	}

	SoundEvent::~SoundEvent()
	{ 
		// stop the source voice
		sourceVoice->Stop();

		// delete the source voice
		sourceVoice->DestroyVoice();

		// clear the buffers
		ZeroMemory(&audioBuffer, sizeof(XAUDIO2_BUFFER));
		audioData.clear();

		// clear the wave format
		ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
	}


	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////// Load Files //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioComponent::loadFile(const std::wstring fileName, SoundEvent& soundEvent, const AudioTypes& soundType)
	{
		// handle errors
		util::Expected<void> result;
		HRESULT hr = S_OK;

		// load file into wave
		WAVEFORMATEX* waveFormatEx;
		result = engine->loadFile(fileName, soundEvent.audioData, &waveFormatEx, soundEvent.waveLength);
		if (!result.isValid())
			return result;
		soundEvent.waveFormat = *waveFormatEx;
		
		// create source voice
		if(soundType == AudioTypes::Sound)
			hr = engine->dev->CreateSourceVoice(&soundEvent.sourceVoice, &soundEvent.waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &soundsSendList, NULL);
		else if(soundType == AudioTypes::Music)
			hr = engine->dev->CreateSourceVoice(&soundEvent.sourceVoice, &soundEvent.waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &musicSendList, NULL);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create source voice!");

		ZeroMemory(&soundEvent.audioBuffer, sizeof(XAUDIO2_BUFFER));
		soundEvent.audioBuffer.AudioBytes = (UINT32)soundEvent.audioData.size();
		soundEvent.audioBuffer.pAudioData = (BYTE* const)&soundEvent.audioData[0];
		soundEvent.audioBuffer.pContext = nullptr;

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////// Stream //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioComponent::streamFile(const std::wstring fileName, const AudioTypes type, const bool loop)
	{ 
		engine->stopStreaming = false;
		if(type == AudioTypes::Music)
			streamingThread = new std::thread(&AudioEngine::streamFile, engine, fileName, musicSendList, loop);
		else if(type == AudioTypes::Sound)
			streamingThread = new std::thread(&AudioEngine::streamFile, engine, fileName, soundsSendList, loop);

		// return success
		return { };
	}

	void AudioComponent::endStream()
	{
		engine->stopStreaming = true;

		if (streamingThread->joinable())
			streamingThread->join();
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////// Play //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioComponent::playSoundEvent(const SoundEvent& audioEvent)
	{
		// handle errors
		HRESULT hr = S_OK;

		// submit the audio buffer to the source voice
		hr = audioEvent.sourceVoice->SubmitSourceBuffer(&audioEvent.audioBuffer);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to submit source buffer!");

		// start the source voice
		audioEvent.sourceVoice->Start();
		
		// return success
		return { };
	}

	util::Expected<void> AudioComponent::stopSoundEvent(const SoundEvent& audioEvent)
	{
		audioEvent.sourceVoice->Stop();

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////// Messages //////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioComponent::onMessage(const core::Depesche& depesche)
	{
		if (depesche.type == core::DepescheTypes::PlaySoundEvent)
		{
			// handle errors
			HRESULT hr = S_OK;

			if (depesche.message == nullptr)
				return std::runtime_error("Critical error: depesche was empty!");

			// submit the audio buffer to the source voice
			hr = ((SoundEvent*)depesche.message)->sourceVoice->SubmitSourceBuffer(&((SoundEvent*)depesche.message)->audioBuffer);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to submit source buffer!");

			// start the source voice
			((SoundEvent*)depesche.message)->sourceVoice->Start();
		}
		else if (depesche.type == core::DepescheTypes::StopSoundEvent)
		{
			if (depesche.message == nullptr)
				return std::runtime_error("Critical error: depesche was empty!");
			
			// simply stop the source voice
			((SoundEvent*)depesche.message)->sourceVoice->Stop();
		}
		else if (depesche.type == core::DepescheTypes::BeginStream)
		{
			// begin streaming music
			if (depesche.message == nullptr)
				return std::runtime_error("Critical error: depesche was empty!");

			util::Expected<void> result = streamFile(((StreamEvent*)depesche.message)->filename, ((StreamEvent*)depesche.message)->type, ((StreamEvent*)depesche.message)->loop);
			if (!result.isValid())
				return result;
		}
		else if (depesche.type == core::DepescheTypes::EndStream)
			endStream();
		
		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////// Volume ////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	void AudioComponent::setVolume(const AudioTypes& audioType, const float volume)
	{
		if (audioType == AudioTypes::Music)
		{
			musicVolume = volume;
			musicSubmix->SetVolume(volume);
		}
		else if (audioType == AudioTypes::Sound)
		{
			soundEffectsVolume = volume;
			soundsSubmix->SetVolume(volume);
		}
	}

	const float AudioComponent::getVolume(const AudioTypes& audioType) const
	{
		if (audioType == AudioTypes::Music)
			return musicVolume;
		else if (audioType == AudioTypes::Sound)
			return soundEffectsVolume;

		return 1.0f;
	}

	// load volume from preference file
	util::Expected<void> AudioComponent::loadVolume()
	{
		if (dxApp.getFileSystemComponent().hasValidConfigurationFile())
		{
			// configuration file exists, try to read from it
			std::wstring pathToPrefFile = dxApp.getFileSystemComponent().getPathToConfigurationFiles() + L"\\bell0prefs.lua";

			try
			{
				sol::state lua;
				lua.script_file(util::StringConverter::ws2s(pathToPrefFile));

				// read from the configuration file, default to 1920 x 1080
				musicVolume = lua["config"]["musicVolume"].get_or(1.0f);
				soundEffectsVolume = lua["config"]["soundEffectsVolume"].get_or(1.0f);
#ifndef NDEBUG
				std::stringstream res;
				res << "The volume was read from the Lua configuration file: " << musicVolume << " x " << soundEffectsVolume << ".";
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>(res.str());
#endif
			}
			catch (std::exception)
			{
				util::ServiceLocator::getFileLogger()->print<util::SeverityType::warning>("Unable to read configuration file. Starting with default volume: 1.0f");
			}
		}

		// return success
		return { };
	}
}