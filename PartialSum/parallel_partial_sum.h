#pragma once

#include <iterator>
#include <thread>
#include <vector>

#include "barrier.h"

class join_threads
{
	std::vector<std::thread>& threads;

public:
	join_threads(std::vector<std::thread>& threads_) :
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

// Listing 8.13
template<typename Iterator>
void parallel_partial_sum(Iterator first, Iterator last)
{
	using value_type = typename Iterator::value_type;

	struct process_element
	{
		void operator() (Iterator first, Iterator last,
			std::vector<value_type>& buffer,
			unsigned i,
			barrier& b)
		{
			value_type& ith_element = *(first + i);
			bool update_source = false;

			for (unsigned step = 0, stride = 1; stride <= i; ++step, stride *= 2)
			{
				const value_type& source = (step % 2) ? buffer[i] : ith_element;
				value_type& dest = (step % 2) ? ith_element : buffer[i];
				const value_type& addend = (step % 2) ? buffer[i - stride] : *(first + i - stride);
				dest = source + addend;
				update_source = !(step % 2);
				b.wait();
			}

			if (update_source)
			{
				ith_element = buffer[i];
			}

			b.done_waiting();
		}
	};

	const unsigned length = std::distance(first, last);

	if (length <= 1)
	{
		return;
	}

	std::vector<value_type> buffer(length);
	barrier b(length);
	std::vector<std::thread> threads(length - 1);
	join_threads joiner(threads);

	buffer[0] = *first;
	for (unsigned i = 0; i < (length - 1); ++i)
	{
		threads[i] = std::thread(process_element(), first, last, std::ref(buffer),
			i, std::ref(b));
	}

	process_element()(first, last, buffer, length - 1, b);
}