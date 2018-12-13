#pragma once

/****************************************************************************************
* Author:	Gilles Bellot
* Date:		28/06/2018 - Lenningen - Luxembourg
*
* Desc:		a thread-safe queue to be used in the event pattern
*
* History:
****************************************************************************************/

// INCLUDES /////////////////////////////////////////////////////////////////////////////

// c++ includes
#include <queue>
#include <mutex>
#include <condition_variable>

// CLASSES //////////////////////////////////////////////////////////////////////////////
namespace util
{
	template<class T>
	class ThreadSafeQueue
	{
	public:
		// constructor and destructor
		ThreadSafeQueue() : queue(), mutex(), condition() {};
		~ThreadSafeQueue() {};

		// add a message to the queue
		void enqueue(T& t)
		{
			// lock the mutex
			std::lock_guard<std::mutex> lock(mutex);

			// push the element to the queue
			queue.push(t);

			// unlock the thread
			condition.notify_one();
		}

		// get the front message from the queue
		// if the queue is empty, wait until a message is available
		const T dequeue()
		{
			std::unique_lock<std::mutex> lock(mutex);
			//while (queue.empty())
			//	// release the lock while waiting
			//	condition.wait(lock);

			if (!queue.empty())
			{
				T message = queue.front();
				queue.pop();
				return message;
			}
			else
				return T();
		}

		// do nothing if the queue is empty
		const bool isEmpty() const
		{
			return queue.empty();
		}
		
	private:
		std::queue<T> queue;				// the actual queue
		mutable std::mutex mutex;			// the mutex (basically telling which thread is allowed to access the queue)
		std::condition_variable condition;	// block the calling thread until notified to resume
	};
}
