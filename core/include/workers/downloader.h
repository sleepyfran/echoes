#pragma once

#include "concurrency.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace downloader
{
enum class DownloadFailureReason : uint8_t
{
    NetworkError,
    InvalidUrl,
    ServerError,
};

using DownloadResult = std::variant<std::string, DownloadFailureReason>;

struct DownloadJob
{
    std::string url;
    /**
     * If specified, instructs the downloader to send the Range header to the server to only
     * download the first N bytes of the file. If the server does not support the Range header, the
     * entire file will be downloaded.
     */
    std::optional<size_t> max_byte_size_to_download;
};

class Downloader : concurrency::Concurrently<DownloadJob, DownloadResult>
{
  private:
    DownloadResult run(const DownloadJob& job) override;
    void on_done() override;

  public:
    Downloader(size_t concurrent_downloads = 4)
        : concurrency::Concurrently<DownloadJob, DownloadResult>(concurrent_downloads) {};

    void queue_download(std::string, size_t, concurrency::OnCompletedCallback<DownloadResult>);
    void wait_for_all_downloads();
};
} // namespace downloader
