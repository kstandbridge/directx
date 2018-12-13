#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		11/07/2018 - Lenningen, Luxembourg
*
* Desc:		audio component
*
* History:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// C++ includes
#include <vector>
#include <thread>

// Microsoft includes
#include <wrl.h>

// XAudio includes
#include <xaudio2.h>

// bell0bytes util
#include "expected.h"
#include "depesche.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace core
{
	class DirectXApp;
}

namespace audio
{
	enum AudioTypes { Music, Sound };
	class AudioEngine;

	// sound event class to store data that can't be stored in RIFF files
	struct SoundEvent
	{
	private:
		IXAudio2SourceVoice* sourceVoice;	// the XAudio2 source voice
		WAVEFORMATEX waveFormat;			// the format of the audio file
		unsigned int waveLength;			// the length of the wave
		std::vector<BYTE> audioData;		// the audio data
		XAUDIO2_BUFFER audioBuffer;			// the actual buffer with the audio data

		float fallof;						// falloff distance
		unsigned int priority;				// music priority
		
		unsigned int index;					// the index of the actual sound to play
	public:
		SoundEvent();
		~SoundEvent();

		friend class AudioComponent;
	};

	struct StreamEvent
	{
		std::wstring filename;
		bool loop = false;
		AudioTypes type = AudioTypes::Music;

		StreamEvent() : filename(L""), loop(false), type(AudioTypes::Music) {};
		StreamEvent(const std::wstring& filename, const bool loop, const AudioTypes type) : filename(filename), loop(loop), type(type) { };
		~StreamEvent() {};

		friend class AudioComponent;
	};

	class AudioComponent : public core::DepescheDestination
	{
	private:
		// address of the main DirectXApp
		const core::DirectXApp& dxApp;

		// the main audio engine
		AudioEngine* engine;				// the main audio engine: XAudio2 with Windows Media Component

		// submix voices
		IXAudio2SubmixVoice* soundsSubmix;	// collection of all sound effects
		IXAudio2SubmixVoice* musicSubmix;	// collection of all music files

		XAUDIO2_SEND_DESCRIPTOR sendSounds;
		XAUDIO2_VOICE_SENDS soundsSendList;

		XAUDIO2_SEND_DESCRIPTOR sendMusic;
		XAUDIO2_VOICE_SENDS musicSendList;

		// streaming
		std::thread* streamingThread;

		// volume
		float soundEffectsVolume = 1.0f;
		float musicVolume = 1.0f;
		
		// handle message
		util::Expected<void> onMessage(const core::Depesche& depesche);

		// load volume from prev file
		util::Expected<void> loadVolume();

	public:
		// constructor and destructor
		AudioComponent(const core::DirectXApp& dxApp);
		~AudioComponent();

		// load file from disk
		util::Expected<void> loadFile(const std::wstring fileName, SoundEvent& soundEvent, const AudioTypes& soundType);
		
		// play sound
		util::Expected<void> playSoundEvent(const SoundEvent& soundEvent);
		util::Expected<void> stopSoundEvent(const SoundEvent& soundEvent);

		// stream files from disk
		util::Expected<void> streamFile(const std::wstring fileName, const AudioTypes type, const bool loop);
		void endStream();

		// volume
		void setVolume(const AudioTypes& audioType, const float volume);
		const float getVolume(const AudioTypes& audioType) const;
	};
}