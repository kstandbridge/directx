// Windows Media Foundation
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid")

// multithreading
#include <thread>

// XAudio2 class
#include "XAudio2.h"

// bell0bytes util
#include "serviceLocator.h"

// bell0bytes audio
#include "audioComponent.h"

namespace audio
{

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	AudioEngine::AudioEngine() : dev(NULL), masterVoice(NULL)
	{
		util::Expected<void> result = initialize();

		if (!result.isValid())
			throw std::runtime_error("Critical error: Unable to initialize the XAudio2 engine!");

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The initialization of the audio component was successful.");
	}
	AudioEngine::~AudioEngine()
	{
		// shut down the media foundation
		MFShutdown();

		// destroy the master voice
		masterVoice->DestroyVoice();

		// stop the engine
		dev->StopEngine();

		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The audio component was successfully destroyed.");
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Initialization ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioEngine::initialize()
	{
		HRESULT hr = S_OK;

		// initialize media foundation
		hr = MFStartup(MF_VERSION);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to start the Windows Media Foundation!");

		// set media foundation reader to low latency
		hr = MFCreateAttributes(sourceReaderConfiguration.GetAddressOf(), 1);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create Media Foundation Source Reader configuration!");
		
		hr = sourceReaderConfiguration->SetUINT32(MF_LOW_LATENCY, true);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to set Windows Media Foundation configuration!");
		
		// get an interface to the main XAudio2 device
		hr = XAudio2Create(dev.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create the XAudio2 engine!");

#ifndef NDEBUG
		// set debugging
		XAUDIO2_DEBUG_CONFIGURATION conf;
		ZeroMemory(&conf, sizeof(XAUDIO2_DEBUG_CONFIGURATION));
		conf.TraceMask = XAUDIO2_LOG_ERRORS;
		dev->SetDebugConfiguration(&conf);
#endif

		// create master voice
		hr = dev->CreateMasteringVoice(&masterVoice);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create the XAudio2 mastering voice!");

		// return success
		return {};
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Load ///////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioEngine::loadFile(const std::wstring& filename, std::vector<BYTE>& audioData, WAVEFORMATEX** waveFormatEx, unsigned int& waveFormatLength)
	{
		// handle errors
		HRESULT hr = S_OK;

		// stream index
		DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

		// the source reader to syncronous mode
		hr = sourceReaderConfiguration->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, NULL);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to set the source reader callback class for syncronous read!");

		// create the source reader
		Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
		hr = MFCreateSourceReaderFromURL(filename.c_str(), sourceReaderConfiguration.Get(), sourceReader.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create source reader from URL!");

		// select the first audio stream, and deselect all other streams
		hr = sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to disable streams!");

		hr = sourceReader->SetStreamSelection(streamIndex, true);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to enable first audio stream!");
		
		// query information about the media file
		Microsoft::WRL::ComPtr<IMFMediaType> nativeMediaType;
		hr = sourceReader->GetNativeMediaType(streamIndex, 0, nativeMediaType.GetAddressOf());
		if(FAILED(hr))
			return std::runtime_error("Critical error: Unable to query media information!");

		// make sure that this is really an audio file
		GUID majorType{};
		hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
		if (majorType != MFMediaType_Audio)
			return std::runtime_error("Critical error: the requested file is not an audio file!");

		// check whether the audio file is compressed or uncompressed
		GUID subType{};
		hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
		if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM)
		{
			// the audio file is uncompressed
		}
		else
		{
			// the audio file is compressed; we have to decompress it first
			// to do so, we inform the SourceReader that we want uncompressed data
			// this causes the SourceReader to look for decoders to perform our request
			Microsoft::WRL::ComPtr<IMFMediaType> partialType = nullptr;
			hr = MFCreateMediaType(partialType.GetAddressOf());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable create media type!");

			// set the media type to "audio"
			hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set media type to audio!");

			// request uncompressed data
			hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set guid of media type to uncompressed!");

			hr = sourceReader->SetCurrentMediaType(streamIndex, NULL, partialType.Get());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set current media type!");
		}

		// uncompress the data and load it into an XAudio2 Buffer
		Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType = nullptr;
		hr = sourceReader->GetCurrentMediaType(streamIndex, uncompressedAudioType.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to retrieve the current media type!");

		hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), waveFormatEx, &waveFormatLength);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create the wave format!");

		// ensure the stream is selected
		hr = sourceReader->SetStreamSelection(streamIndex, true);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to select audio stream!");

		// copy data into byte vector
		Microsoft::WRL::ComPtr<IMFSample> sample = nullptr;
		Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer = nullptr;
		BYTE* localAudioData = NULL;
		DWORD localAudioDataLength = 0;

		while (true)
		{
			DWORD flags = 0;
			hr = sourceReader->ReadSample(streamIndex, 0, nullptr, &flags, nullptr, sample.GetAddressOf());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to read audio sample!");
			
			// check whether the data is still valid
			if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
				break;

			// check for eof
			if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
				break;

			if (sample == nullptr)
				continue;

			// convert data to contiguous buffer
			hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to convert audio sample to contiguous buffer!");

			// lock buffer and copy data to local memory
			hr = buffer->Lock(&localAudioData, nullptr, &localAudioDataLength);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to lock the audio buffer!");
			
			for (size_t i = 0; i < localAudioDataLength; i++)
				audioData.push_back(localAudioData[i]);
			
			// unlock the buffer
			hr = buffer->Unlock();
			localAudioData = nullptr;

			if (FAILED(hr))
				return std::runtime_error("Critical error while unlocking the audio buffer!");
		}

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Stream /////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> AudioEngine::streamFile(const std::wstring& filename, XAUDIO2_VOICE_SENDS sendList, const bool loop)
	{
		// handle errors
		HRESULT hr = S_OK;
		util::Expected<void> result;

		DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

		// create the asyncronous source reader
		Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
		WAVEFORMATEX waveFormat;
		result = createAsyncReader(filename, sourceReader.GetAddressOf(), &waveFormat, sizeof(waveFormat));
		if (!result.isValid())
			return result;

		// create the source voice
		IXAudio2SourceVoice* sourceVoice;
		hr = dev->CreateSourceVoice(&sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &streamingVoiceCallback, &sendList, NULL);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create the source voice for streaming!");
		sourceVoice->Start();

		// loop
		result = loopStream(sourceReader.Get(), sourceVoice, loop);
		if (!result.isValid())
			return result;

		sourceReader->Flush(streamIndex);
		sourceVoice->DestroyVoice();
		sourceReader = nullptr;

		// return success
		return { };
	}

	util::Expected<void> AudioEngine::loopStream(IMFSourceReader* const sourceReader, IXAudio2SourceVoice* const sourceVoice, const bool loop)
	{
		DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;
		DWORD currentStreamBuffer = 0;
		HRESULT hr = S_OK;
		size_t bufferSize[maxBufferCount] = { 0 };
		std::unique_ptr<uint8_t[]> buffers[maxBufferCount];

		for (;;)
		{
			if (stopStreaming)
				break;

			hr = sourceReader->ReadSample(streamIndex, 0, nullptr, nullptr, nullptr, nullptr);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to read source sample!");

			WaitForSingleObject(sourceReaderCallback.hReadSample, INFINITE);

			if (sourceReaderCallback.endOfStream)
			{
				if (loop)
				{
					// restart the stream
					sourceReaderCallback.Restart();
					PROPVARIANT var = { 0 };
					var.vt = VT_I8;
					hr = sourceReader->SetCurrentPosition(GUID_NULL, var);
					if (SUCCEEDED(hr))
						continue;
					else
						return std::runtime_error("Critical error: Unable to set the source reader position!");
				}
				else
					break;
			}

			Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
			hr = sourceReaderCallback.sample->ConvertToContiguousBuffer(mediaBuffer.GetAddressOf());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to convert audio data to contiguous buffer!");

			BYTE* audioData = nullptr;
			DWORD sampleBufferLength = 0;

			hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to create lock the media buffer!");

			if (bufferSize[currentStreamBuffer] < sampleBufferLength)
			{
				buffers[currentStreamBuffer].reset(new uint8_t[sampleBufferLength]);
				bufferSize[currentStreamBuffer] = sampleBufferLength;
			}

			memcpy_s(buffers[currentStreamBuffer].get(), sampleBufferLength, audioData, sampleBufferLength);

			hr = mediaBuffer->Unlock();
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to unlock the media buffer!");

			// wait until the XAudio2 source has played enough data
			// we want to have only maxBufferCount-1 buffers on the queue to make sure that there is always one free buffer for the Media Foundation streamer
			XAUDIO2_VOICE_STATE state;
			for (;;)
			{
				sourceVoice->GetState(&state);
				if (state.BuffersQueued < maxBufferCount - 1)
					break;

				WaitForSingleObject(streamingVoiceCallback.hBufferEndEvent, INFINITE);
			}

			XAUDIO2_BUFFER buf = { 0 };
			buf.AudioBytes = sampleBufferLength;
			buf.pAudioData = buffers[currentStreamBuffer].get();
			sourceVoice->SubmitSourceBuffer(&buf);

			currentStreamBuffer++;
			currentStreamBuffer %= maxBufferCount;
		}

		if(FAILED(hr))
			return std::runtime_error("Critical error: Unable to loop through the media stream!");

		// return success
		return { };
	}

	util::Expected<void> AudioEngine::createAsyncReader(const std::wstring& filename, IMFSourceReader** sourceReader, WAVEFORMATEX* wfx, size_t wfxSize)
	{
		// handle errors
		HRESULT hr = S_OK;

		// set the source reader to asyncronous mode
		hr = sourceReaderConfiguration->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, &sourceReaderCallback);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to set the source reader callback class for asyncronous read!");

		// create the source reader
		hr = MFCreateSourceReaderFromURL(filename.c_str(), sourceReaderConfiguration.Get(), sourceReader);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create source reader from URL!");

		// stream index
		DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

		// select the first audio stream, and deselect all other streams
		hr = (*sourceReader)->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to disable streams!");

		hr = (*sourceReader)->SetStreamSelection(streamIndex, true);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to enable first audio stream!");

		// query information about the media file
		Microsoft::WRL::ComPtr<IMFMediaType> nativeMediaType;
		hr = (*sourceReader)->GetNativeMediaType(streamIndex, 0, nativeMediaType.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to query media information!");

		// make sure that this is really an audio file
		GUID majorType{};
		hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
		if (majorType != MFMediaType_Audio)
			return std::runtime_error("Critical error: the requested file is not an audio file!");

		// check whether the audio file is compressed or uncompressed
		GUID subType{};
		hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
		if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM)
		{
			// the audio file is uncompressed
		}
		else
		{
			// the audio file is compressed; we have to decompress it first
			// to do so, we inform the SourceReader that we want uncompressed data
			// this causes the SourceReader to look for decoders to perform our request
			Microsoft::WRL::ComPtr<IMFMediaType> partialType = nullptr;
			hr = MFCreateMediaType(partialType.GetAddressOf());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable create media type!");

			// set the media type to "audio"
			hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set media type to audio!");

			// request uncompressed data
			hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set guid of media type to uncompressed!");

			hr = (*sourceReader)->SetCurrentMediaType(streamIndex, NULL, partialType.Get());
			if (FAILED(hr))
				return std::runtime_error("Critical error: Unable to set current media type!");
		}
	
		// uncompress the data
		Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType = nullptr;
		hr = (*sourceReader)->GetCurrentMediaType(streamIndex, uncompressedAudioType.GetAddressOf());
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to retrieve the current media type!");

		UINT32 waveFormatSize = 0;
		WAVEFORMATEX* waveFormat = nullptr;
		hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), &waveFormat, &waveFormatSize);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to create the wave format!");

		// ensure the stream is selected
		hr = (*sourceReader)->SetStreamSelection(streamIndex, true);
		if (FAILED(hr))
			return std::runtime_error("Critical error: Unable to select audio stream!");

		// copy data
		memcpy_s(wfx, wfxSize, waveFormat, waveFormatSize);
		CoTaskMemFree(waveFormat);

		// return success
		return { };
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Callback ///////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	StreamingVoiceCallback::StreamingVoiceCallback() : hBufferEndEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE))
	{ }

	StreamingVoiceCallback::~StreamingVoiceCallback()
	{
		CloseHandle(hBufferEndEvent);
	}

	SourceReaderCallback::SourceReaderCallback() : hReadSample(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE)), endOfStream(false), status(S_OK)
	{ }
	SourceReaderCallback::~SourceReaderCallback()
	{
		CloseHandle(hReadSample);
	}

	void SourceReaderCallback::Restart()
	{
		std::lock_guard<std::mutex> lock(guard);
		endOfStream = false;
		sample = nullptr;
	}
}