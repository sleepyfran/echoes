#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <utility>

namespace concurrency
{

template <typename T> using OnCompletedCallback = std::function<void(const T&)>;

template <typename T, typename R> struct JobWithCompletion
{
    T job;
    OnCompletedCallback<R> on_completed;
};

template <typename T, typename R> class Concurrently
{
  private:
    std::mutex mutex_;
    std::condition_variable_any cv_;
    std::condition_variable_any done_cv_;
    size_t active_jobs_ = 0;
    std::queue<JobWithCompletion<T, R>> jobs_;

    std::vector<std::jthread> workers_;

    virtual R run(const T& job) = 0;

    void worker(std::stop_token& stop)
    {
        while (!stop.stop_requested())
        {
            JobWithCompletion<T, R> job_with_completion;

            {
                std::unique_lock lock{mutex_};

                cv_.wait(lock, stop, [&] { return !jobs_.empty(); });

                if (stop.stop_requested())
                {
                    return;
                }

                job_with_completion = std::move(jobs_.front());
                jobs_.pop();

                ++active_jobs_;
            }

            auto result = run(job_with_completion.job);
            job_with_completion.on_completed(result);

            {
                std::unique_lock lock{mutex_};
                --active_jobs_;

                if (jobs_.empty() && active_jobs_ == 0)
                {
                    done_cv_.notify_all();
                }
            }
        }
    }

  public:
    Concurrently(size_t concurrency = 4)
    {
        this->workers_.reserve(concurrency);

        for (std::size_t i = 0; i < concurrency; ++i)
        {
            workers_.emplace_back([this](std::stop_token stop) { worker(stop); });
        }
    }

    virtual ~Concurrently() = default;

    /**
     * Queues a new job to be processed by one of the workers.
     */
    void queue_job(const T& job, OnCompletedCallback<R> on_completed)
    {

        {
            std::lock_guard lock{mutex_};
            jobs_.push(JobWithCompletion<T, R>{.job = job, .on_completed = on_completed});
        }

        cv_.notify_one();
    }

    /**
     * Waits for all queued downloads to complete. This method blocks until all jobs have
     * completed or failed.
     */
    void wait_for_all_queued()
    {

        std::unique_lock lock{mutex_};

        done_cv_.wait(lock, [this] { return jobs_.empty() && active_jobs_ == 0; });
    }
};
} // namespace concurrency
