#pragma once

#include <algorithm>
#include <future>
#include <iterator>
#include <thread>
#include <vector>

// Code sample on p. 275
class join_threads
{
	std::vector<std::thread>& threads;

public:
	explicit join_threads(std::vector<std::thread>& threads_):
		threads(threads_)
	{}
	~join_threads()
	{
		for (auto& t : threads)
		{
			if (t.joinable())
			{
				t.join();
			}
		}
	}
};

// Listing 8.7
// std::packaged_task and std::future are used to transfer the exception between threads
template<typename Iterator, typename Func>
void parallel_for_each(Iterator first, Iterator last, Func f)
{
	const unsigned length = std::distance(first, last);

	if (!length)
	{
		return;
	}

	// can be lower for testing
	const unsigned min_per_thread = 25;
	const unsigned max_threads = (length + min_per_thread - 1) / min_per_thread;
	const auto hardware_threads = std::thread::hardware_concurrency();
	const auto num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
	const auto block_size = length / num_threads;
	// -1 because we also do some work on the current thread
	std::vector<std::future<void>> futures(num_threads - 1);
	std::vector<std::thread> threads(num_threads - 1);
	// join RAII guard
	join_threads joiner(threads);
	auto block_start = first;

	for (unsigned i = 0; i < (num_threads - 1); ++i)
	{
		auto block_end = block_start;
		std::advance(block_end, block_size);
		// create an async function out of a regular one
		std::packaged_task<void(void)> task(
			[=]()
			{
				std::for_each(block_start, block_end, f);
			});
		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task));
		block_start = block_end;
	}

	std::for_each(block_start, last, f);

	// Check for exceptions, can be ommited otherwise
	for (unsigned i = 0; i < (num_threads - 1); ++i)
	{
		futures[i].get();
	}
}