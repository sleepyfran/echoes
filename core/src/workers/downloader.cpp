#include "workers/downloader.h"
#include "httplib.hpp"
#include "utils.h"

namespace downloader
{
DownloadResult Downloader::run(const DownloadJob& job)
{
    auto parsed_url = utils::parse_url(job.url);
    if (!parsed_url)
    {
        return DownloadFailureReason::InvalidUrl;
    }

    httplib::SSLClient client{parsed_url->host};

    httplib::Headers headers{};
    if (job.max_byte_size_to_download)
    {
        headers.emplace(
            "Range",
            std::format("bytes=0-{}", std::to_string(job.max_byte_size_to_download.value())));
    }

    auto response = client.Get(parsed_url->path, headers);

    if (!response)
    {
        return DownloadFailureReason::NetworkError;
    }

    if (response->status != 200 && response->status != 206)
    {
        return DownloadFailureReason::ServerError;
    }

    return response->body;
}

void Downloader::queue_download(std::string url, size_t max_byte_size_to_download,
                                concurrency::OnCompletedCallback<DownloadResult> on_completed)
{
    this->queue_job(
        DownloadJob{.url = std::move(url), .max_byte_size_to_download = max_byte_size_to_download},
        std::move(on_completed));
}

void Downloader::wait_for_all_downloads()
{
    this->wait_for_all_queued();
}
} // namespace downloader
