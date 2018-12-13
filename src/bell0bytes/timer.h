#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		03/07/2017 - Dortmund - Germany
*
* Desc:		A high-perfomance timer powered by the Windows perfomance counter
*
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// CLASSES //////////////////////////////////////////////////////////////////////////////

namespace util
{
	template<typename T>
	class Expected;
}

namespace core
{
	class Timer
	{
	private:
		// times measured in counts
		long long int startTime;			// time at the start of the application
		long long int totalIdleTime;		// total time the game was idle
		long long int pausedTime;			// time at the moment the game was paused last
		long long int currentTime;			// stores the current time; i.e. time at the current frame
		long long int previousTime;		    // stores the time at the last inquiry before current; i.e. time at the previous frame

		// times measured in seconds
		double secondsPerCount;			    // reciprocal of the frequency, computed once at the initialization of the class
		double deltaTime;					// time between two frames, updated during the game loop

		// state of the timer
		bool isStopped;					    // true iff the timer is stopped

	public:
		// constructor
		Timer();
		~Timer();

		// getters: return time measured in seconds
		double getTotalTime() const;		// returns the total time the game has been running (minus paused time)
		double getDeltaTime() const;		// returns the time between two frames

		// methods
		util::Expected<void> start();		// starts the timer, called each time the game is unpaused
		util::Expected<void> reset();		// sets the counter to zero, called once before message loop
		util::Expected<void> tick();		// called every frame, lets the time tick
		util::Expected<void> stop();		// called when the game is paused
	};
}