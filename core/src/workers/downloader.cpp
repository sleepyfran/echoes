#include "workers/downloader.h"
#include "httplib.hpp"
#include "utils.h"

namespace downloader
{
Downloader::Downloader(size_t concurrent_downloads)
{
    this->workers_.reserve(concurrent_downloads);

    for (std::size_t i = 0; i < concurrent_downloads; ++i)
    {
        workers_.emplace_back([this](std::stop_token stop) { worker(stop); });
    }
}

void Downloader::queue(const std::string& url, OnCompletedCallback on_completed)
{
    {
        std::lock_guard lock{mutex_};
        jobs_.push({.url = url, .on_completed = std::move(on_completed)});
    }

    cv_.notify_one();
}

void Downloader::wait_for_all_queued()
{
    std::unique_lock lock{mutex_};

    done_cv_.wait(lock, [this] { return jobs_.empty() && active_jobs_ == 0; });
}

void download(const DownloadJob& job)
{
    auto parsed_url = utils::parse_url(job.url);
    if (!parsed_url)
    {
        job.on_completed(DownloadFailureReason::InvalidUrl);
        return;
    }

    httplib::SSLClient client{parsed_url->host};
    httplib::Headers headers{{"Range", "bytes=0-50000"}};
    auto response = client.Get(parsed_url->path, headers);

    if (!response)
    {
        job.on_completed(DownloadFailureReason::NetworkError);
        return;
    }

    if (response->status != 200 && response->status != 206)
    {
        job.on_completed(DownloadFailureReason::ServerError);
        return;
    }

    job.on_completed(response->body);
}

void Downloader::worker(std::stop_token& stop)
{
    while (!stop.stop_requested())
    {
        DownloadJob job;

        {
            std::unique_lock lock{mutex_};

            cv_.wait(lock, stop, [&] { return !jobs_.empty(); });

            if (stop.stop_requested())
            {
                return;
            }

            job = std::move(jobs_.front());
            jobs_.pop();

            ++active_jobs_;
        }

        download(job);

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
} // namespace downloader
