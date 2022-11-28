#include "ThreadSafeStack.cpp"
#include <list>
#include <future>
#include <thread>
#include <atomic>
#include <algorithm>

template<typename T>
struct sorter
{
	struct chunk_to_sort
	{
		std::list<T> data;
		std::promise<std::list<T> > promise;
	};
	threadsafe_stack<chunk_to_sort> chunks;
	std::vector<std::thread> threads;
	unsigned const max_thread_count;
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
		try {
			std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
			if (chunk)
			{
				sort_chunk(chunk);
			}
		}
		catch (...) {
			printf("Stack is empty");
		}
		
	}
	std::list<T> do_sort(std::list<T>& chunk_data)
	{
		if (chunk_data.empty())
		{
			return chunk_data;
		}
		std::list<T> result;
		result.splice(result.begin(), chunk_data, chunk_data.begin());
		T const& partition_val = *result.begin();
		typename std::list<T>::iterator divide_point =
			std::partition(chunk_data.begin(), chunk_data.end(),
				[&](T const& val) {return val < partition_val; });
		chunk_to_sort new_lower_chunk;
		new_lower_chunk.data.splice(new_lower_chunk.data.end(),
			chunk_data, chunk_data.begin(),
			divide_point);
		std::future<std::list<T> > new_lower =
			new_lower_chunk.promise.get_future();
		chunks.push(std::move(new_lower_chunk));
		if (threads.size() < max_thread_count)
		{
			threads.push_back(std::thread(&sorter<T>::sort_thread, this));
		}
		std::list<T> new_higher(do_sort(chunk_data));
		result.splice(result.end(), new_higher);
		while (new_lower.wait_for(std::chrono::seconds(0)) !=
			std::future_status::ready)
		{
			try_sort_chunk();
		}
		result.splice(result.begin(), new_lower.get());
		return result;
	}
	void sort_chunk(std::shared_ptr<chunk_to_sort > const& chunk)
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