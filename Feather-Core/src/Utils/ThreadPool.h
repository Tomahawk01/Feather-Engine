#pragma once

#include <queue>
#include <condition_variable>
#include <future>

namespace Feather {

	class ThreadPool
	{
	public:
		explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency())
			: m_Stopped{ false }
		{
			for (size_t i = 0; i < threadCount; i++)
			{
				m_Workers.emplace_back(
				[this] {
					while (true)
					{
						std::function<void()> task;

						{
							std::unique_lock lock(m_QueueMutex);
							m_Condition.wait(lock, [this] { return m_Stopped || !m_Tasks.empty(); });

							if (m_Stopped && m_Tasks.empty())
								return;

							task = std::move(m_Tasks.front());

							m_Tasks.pop();
						}

						task();
					}
				});
			}
		}

		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		~ThreadPool()
		{
			{
				std::lock_guard lock(m_QueueMutex);
				m_Stopped = true;
			}

			m_Condition.notify_all();

			for (auto& thread : m_Workers)
			{
				if (thread.joinable())
					thread.join();
			}
		}

		template <typename Func, typename... Args>
		auto Enqueue(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func, Args...>>
		{
			using ReturnType = std::invoke_result_t<Func, Args...>;

			auto taskPtr = std::make_shared<std::packaged_task<ReturnType()>>(
				std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

			{
				std::lock_guard lock(m_QueueMutex);

				if (m_Stopped)
				{
					throw std::runtime_error("ThreadPool is stopped");
				}

				m_Tasks.emplace([taskPtr]() { (*taskPtr)(); });
			}

			m_Condition.notify_one();

			return taskPtr->get_future();
		}

	private:
		std::vector<std::thread> m_Workers;
		std::queue<std::function<void()>> m_Tasks;
		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;
		std::atomic<bool> m_Stopped;
	};

}

using SharedThreadPool = std::shared_ptr<Feather::ThreadPool>;
