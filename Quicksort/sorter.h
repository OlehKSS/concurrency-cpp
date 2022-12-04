#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <list>
#include <memory>
#include <mutex>
#include <stack>
#include <thread>

// dummy implementation
template<typename Value>
class thread_safe_stack
{
private:
	std::stack<std::shared_ptr<Value>> data;
	mutable std::mutex mutex;

public:
	void push(std::shared_ptr<Value> value)
	{
		std::lock_guard<std::mutex> g(mutex);
		data.push(std::move(value));
	}

	std::shared_ptr<Value> pop()
	{
		std::lock_guard<std::mutex> g(mutex);

		if (data.empty())
		{
			return std::make_shared<Value>();
		}

		auto value = data.top();
		data.pop();

		return value;
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> g(mutex);
		return data.empty();
	}
};

template<typename T>
struct sorter
{
	struct chunk_to_sort
	{
		std::list<T> data;
		std::promise<decltype(data)> promise;
	};

	thread_safe_stack<chunk_to_sort> chunks;
	std::vector<std::thread> threads;
	const unsigned max_thread_count;
	std::atomic<bool> end_of_data;

	sorter() :
		max_thread_count(std::thread::hardware_concurrency() - 1),
		end_of_data(false)
	{}

	~sorter()
	{
		end_of_data = true;
		for (unsigned i = 0; i < threads.size(); ++i)
		{
			threads[i].join();
		}
	}

	void try_sort_chunk()
	{
		auto chunk = chunks.pop();

		if (chunk)
		{
			sort_chunk(chunk);
		}
	}

	std::list<T> do_sort(std::list<T>& chunk_data)
	{
		if (chunk_data.empty())
		{
			return chunk_data;
		}
		
		std::list<T> result;
		// transfer 1st element
		result.splice(result.begin(), chunk_data, chunk_data.begin());
		const auto& partition_val = *result.begin();
		auto divide_point = std::partition(chunk_data.begin(), chunk_data.end(),
			[&partition_val](const T& val) { return val < partition_val; });

		auto new_lower_chunk = std::make_shared<chunk_to_sort>();
		new_lower_chunk->data.splice(new_lower_chunk->data.end(),
			chunk_data, chunk_data.begin(), divide_point);

		auto new_lower = new_lower_chunk->promise.get_future();
		chunks.push(new_lower_chunk);

		if (threads.size() < max_thread_count)
		{
			threads.push_back(std::thread(&sorter<T>::sort_thread, this));
		}

		auto new_higher = do_sort(chunk_data);
		result.splice(result.end(), new_higher);

		while (new_lower.wait_for(std::chrono::seconds(0)) !=
			std::future_status::ready)
		{
			try_sort_chunk();
		}
		result.splice(result.begin(), new_lower.get());
		return result;
	}

	void sort_chunk(const std::shared_ptr<chunk_to_sort>& chunk)
	{
		chunk->promise.set_value(do_sort(chunk->data));
	}

	void sort_thread()
	{
		while (!end_of_data)
		{
			try_sort_chunk();
			std::this_thread::yield();
		}
	}
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
	if (input.empty())
	{
		return input;
	}

	sorter<T> s;
	return s.do_sort(input);
}

