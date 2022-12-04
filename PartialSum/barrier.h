#pragma once
#include <atomic>
#include <thread>

// Listing 8.13
struct barrier
{
	std::atomic<unsigned> count;
	std::atomic<unsigned> spaces;
	std::atomic<unsigned> generation;

	barrier(unsigned count_) :
		count(count_), spaces(count_), generation(0)
	{}

	void wait()
	{
		const unsigned gen = generation.load();

		--spaces;
		
		if (!spaces)
		{
			spaces = count.load();
			++generation;
		}
		else
		{
			while (generation.load() == gen)
			{
				std::this_thread::yield();
			}
		}
	}

	void done_waiting()
	{
		--count;
		--spaces;
		if (!spaces)
		{
			spaces = count.load();
			++generation;
		}
	}
};