// INCLUDES /////////////////////////////////////////////////////////////////////////////

// windows
#include <Windows.h>

// bell0bytes core
#include "timer.h"

// bell0bytes util
#include "serviceLocator.h"
#include "expected.h"

namespace core
{
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Constructor //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	Timer::Timer() : startTime(0), totalIdleTime(0), pausedTime(0), currentTime(0), previousTime(0), secondsPerCount(0.0), deltaTime(0.0), isStopped(false)
	{
		// get the frequency of the PerformanceCounter
		long long int frequency = 0;
		if (QueryPerformanceFrequency((LARGE_INTEGER*)&frequency))
		{
			// compute the secondsPerCount as the reciprocal of the frequency
			secondsPerCount = 1.0 / (double)frequency;
#ifndef NDEBUG
			// log success
			util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The high-precision timer was created successfully.");
#endif
		}
		else
			// the hardware does not support a high-precision timer -> throw an error
			throw std::runtime_error("The hardware does not support a high-precision timer!");
	}

	Timer::~Timer()
	{
#ifndef NDEBUG
		// log success
		util::ServiceLocator::getFileLogger()->print<util::SeverityType::info>("The timer was successfully destroyed.");
#endif
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// Getters //////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	double Timer::getDeltaTime() const
	{
		// this function returns the time elapsed between two frames; delta time is updated during the game loop
		return deltaTime;
	}

	double Timer::getTotalTime() const
	{
		// this function returns the total time since the game started: (t_now - t_start) - t_totalIdle
		if (isStopped)
			return (pausedTime - startTime - totalIdleTime)*secondsPerCount;
		else
			return (currentTime - startTime - totalIdleTime)*secondsPerCount;
	}
	/////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////// Timer Lopgic /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////
	util::Expected<void> Timer::reset()
	{
		// this function resets the timer
		long long int now = 0;
		if (QueryPerformanceCounter((LARGE_INTEGER*)&now))
		{
			startTime = now;
			previousTime = now;
			pausedTime = 0;
			isStopped = false;

			// return success
			return {};
		}
		else
			// unable to query the performance counter, throw an error
			return std::runtime_error("Unable to query the performance counter!");
	}
	
	util::Expected<void> Timer::start()
	{
		// this function starts the timer (if it is not already running)
		if (isStopped)
		{
			long long int now = 0;
			if (QueryPerformanceCounter((LARGE_INTEGER*)&now))
			{
				// add the duration of the pause to the total idle time
				totalIdleTime += (now - pausedTime);

				// set the previous time to the current time
				previousTime = now;

				// reset the pauseTime to 0 and isStopped to false
				pausedTime = 0;
				isStopped = false;

				// return success
				return {};
			}
			else
				// unable to query the performance counter, throw an error
				return std::runtime_error("Unable to query the performance counter!");
		}

		// return success
		return {};
	}

	util::Expected<void> Timer::stop()
	{
		// this function stops the timer (if it is currently running)
		if (!isStopped)
		{
			long long int now = 0;
			if (QueryPerformanceCounter((LARGE_INTEGER*)&now))
			{
				// set the time the timer was stopped to "now"
				pausedTime = now;
				isStopped = true;

				// return success
				return {};
			}
			else
				// unable to query the performance counter, throw an error
				return std::runtime_error("Unable to query the performance counter!");
		}

		// return success
		return {};
	}

	util::Expected<void> Timer::tick()
	{
		// this function lets the timer tick, i.e. it computes the time that has elapsed between two frames
		if (isStopped)
		{
			// if the game is stopped, the elapsed time is obviously 0
			deltaTime = 0.0;

			// return success
			return {};
		}
		else
		{
			// get the current time
			if (QueryPerformanceCounter((LARGE_INTEGER*)&currentTime))
			{
				// compute the time elapsed since the previous frame
				deltaTime = (currentTime - previousTime) * secondsPerCount;

				// set previousTime to currentTime, as in the next tick, this frame will be the previous frame
				previousTime = currentTime;

				// deltaTime can be negative if the processor goes idle for example
				if (deltaTime < 0.0)
					deltaTime = 0.0;

				// return success
				return {};
			}
			else
				// unable to query the performance counter, throw an error
				return std::runtime_error("Unable to query the performance counter!");
		}
	}
}