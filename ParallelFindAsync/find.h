#pragma once
#include <atomic>
#include <future>
#include <iterator>

template<typename Iterator, typename MatchType>
Iterator parallel_find_impl(Iterator first, Iterator last, MatchType match,
	std::atomic<bool>& done)
{
	try
	{
		const unsigned length = std::distance(first, last);
		const unsigned min_per_thread = 10;

		if (length < (2 * min_per_thread))
		{
			// Break of recursion
			for (; (first != last) && !done.load(); ++first)
			{
				if (*first == match)
				{
					done = true;
					return first;
				}
			}

			return last;
		}
		else
		{
			const auto mid_point = first + (length / 2);
			// async part
			// std::ref wrapps references into a copyable object, that can be turned into ref again
			auto async_result = std::async(&parallel_find_impl<Iterator, MatchType>,
				mid_point, last, match, std::ref(done));
			// sync part
			const auto direct_result = parallel_find_impl(first, mid_point, match, done);

			return (direct_result == mid_point) ? async_result.get() : direct_result;
		}
	}
	catch (...)
	{
		done = true;
		throw;
	}
}

template<typename Iterator, typename MatchType>
Iterator parallel_find(Iterator first, Iterator last, MatchType match)
{
	std::atomic<bool> done(false);
	return parallel_find_impl(first, last, match, done);
}
