#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
#include <string>
#include <thread>
#include <variant>

namespace downloader
{
enum class DownloadFailureReason : uint8_t
{
    NetworkError,
    InvalidUrl,
    ServerError,
};

using DownloadResult = std::variant<std::string_view, DownloadFailureReason>;

using OnCompletedCallback = std::function<void(const DownloadResult& result)>;

struct DownloadJob
{
    std::string url;
    OnCompletedCallback on_completed;
    /**
     * If specified, instructs the downloader to send the Range header to the server to only
     * download the first N bytes of the file. If the server does not support the Range header, the
     * entire file will be downloaded.
     */
    std::optional<size_t> max_size_to_download;
};

class Downloader
{
  private:
    void worker(std::stop_token& stop);

    std::mutex mutex_;
    std::condition_variable_any cv_;
    std::condition_variable_any done_cv_;
    size_t active_jobs_ = 0;
    std::queue<DownloadJob> jobs_;

    std::vector<std::jthread> workers_;

  public:
    explicit Downloader(size_t concurrent_downloads = 4);

    /**
     * Queues a download of the given URL. The on_completed callback will be called when the
     * download is completed or failed with a reason.
     */
    void queue(const std::string& url, OnCompletedCallback on_completed);

    /**
     * Waits for all queued downloads to complete. This method blocks until all downloads are
     * completed or failed. This method does not stop the downloader, it only waits for the
     * current queued downloads to complete. New downloads can still be queued after this method is
     * called.
     */
    void wait_for_all_queued();
};
} // namespace downloader
