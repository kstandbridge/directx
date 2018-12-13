#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		11/07/2018 - Lenningen, Luxembourg
*
* Desc:		XAudio2
*
* History:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// C++ includes
#include <vector>
#include <mutex>

// Windows includes
#include <wrl.h>

// XAudio2 includes
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

// Windows Media Foundation includes
#include <mfidl.h>
#include <mfreadwrite.h>

// bell0bytes util
#include "expected.h"

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace audio
{
	class AudioComponent;
	struct SoundEvent;

	// callback structure for XAudio2 voices
	struct StreamingVoiceCallback : public IXAudio2VoiceCallback
	{
		HANDLE hBufferEndEvent;

		STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override { };
		STDMETHOD_(void, OnVoiceProcessingPassEnd)() override { };
		STDMETHOD_(void, OnStreamEnd)() override { };
		STDMETHOD_(void, OnBufferStart)(void*) override { };
		STDMETHOD_(void, OnBufferEnd)(void*) override { SetEvent(hBufferEndEvent); };
		STDMETHOD_(void, OnLoopEnd)(void*) override { };
		STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override { };

		StreamingVoiceCallback();
		virtual ~StreamingVoiceCallback();
	};

	// callback structure for the WMF Source Reader
	struct SourceReaderCallback : public IMFSourceReaderCallback
	{
		Microsoft::WRL::ComPtr<IMFSample>	sample;				// the sample that was read in
		HANDLE								hReadSample;		// handle to the read sample
		bool								endOfStream;		// true iff the end of the file was reached
		HRESULT								status;				// the status of the stream
		std::mutex							guard;				// thread-safety

		STDMETHOD(QueryInterface) (REFIID iid, _COM_Outptr_ void** ppv) override 
		{
			if (!ppv)
				return E_POINTER;

			if (_uuidof(IMFSourceReaderCallback) == iid)
			{
				*ppv = this;
				return S_OK;
			}

			*ppv = nullptr;
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)() override { return 1; }
		STDMETHOD_(ULONG, Release)() override { return 1; }
		STDMETHOD(OnReadSample)(_In_ HRESULT hrStatus, _In_ DWORD dwStreamIndex, _In_ DWORD dwStreamFlags, _In_ LONGLONG llTimestamp, _In_opt_ IMFSample *pSample) override
		{
			UNREFERENCED_PARAMETER(dwStreamIndex);
			UNREFERENCED_PARAMETER(llTimestamp);

			std::lock_guard<std::mutex> lock(guard);

			if (SUCCEEDED(hrStatus))
			{
				if (pSample)
				{
					sample = pSample;
				}
			}

			if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
			{
				endOfStream = true;
			}

			status = hrStatus;
			SetEvent(hReadSample);

			return S_OK;
		}
		STDMETHOD(OnFlush)(_In_ DWORD) override { return S_OK; };
		STDMETHOD(OnEvent)(_In_ DWORD, _In_ IMFMediaEvent *) override { return S_OK; };

		void Restart();

		SourceReaderCallback();
		virtual ~SourceReaderCallback();
	};

	// the main audio engine powered by XAudio2 and Windows Media Foundation
	class AudioEngine
	{
	private:
		Microsoft::WRL::ComPtr<IXAudio2> dev;							// the main XAudio2 engine
		IXAudio2MasteringVoice* masterVoice;							// a mastering voice
		Microsoft::WRL::ComPtr<IMFAttributes> sourceReaderConfiguration;// Windows Media Foundation Source Reader Configuration

		// streaming variables
		SourceReaderCallback sourceReaderCallback;						// callback structure for the source reader
		StreamingVoiceCallback streamingVoiceCallback;					// callback structure for the source voice
		static const int maxBufferCount = 3;							// maximal numbers of buffers used during streaming
		bool stopStreaming = false;										// breaks the streaming thread
		
		// initialization
		util::Expected<void> initialize();								// this function initializes the XAudio2 interface
		
		// read audio data from the harddrive
		util::Expected<void> loadFile(const std::wstring& filename, std::vector<BYTE>& audioData, WAVEFORMATEX** wafeFormatEx, unsigned int& waveLength);	// load audio file from disk
		
		// stream audio
		util::Expected<void> createAsyncReader(const std::wstring& filename, IMFSourceReader** sourceReader, WAVEFORMATEX* wfx, size_t wfxSize);			// creates a source reader in asyncrononous mode
		util::Expected<void> streamFile(const std::wstring& filename, XAUDIO2_VOICE_SENDS sendList, const bool loop = false);								// streams a file from the harddrive
		util::Expected<void> loopStream(IMFSourceReader* const sourceReader, IXAudio2SourceVoice* const sourceVoice, const bool loop = false);				// the actual loop of the streaming function

	public:
		// constructor and destructor
		AudioEngine();
		~AudioEngine();

		friend class AudioComponent;
	};
}