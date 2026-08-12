#include "workers/concurrent_file_discovery.h"
#include "concurrency.h"
#include "utils.h"

namespace file_discovery
{
entities::ProviderError map_to_provider_error(const media_provider::MediaProviderError& error)
{
    switch (error)
    {
    case media_provider::MediaProviderError::Unauthorized:
        return entities::ProviderError::TokenExpired;
    default:
        return entities::ProviderError::ApiGatewayError;
    }
}

std::monostate ConcurrentFileDiscovery::run(const FileDiscoveryJob& job)
{
    auto contents = provider.list_folder(job.folder);
    if (contents.status != media_provider::MediaProviderResultStatus::Ok)
    {
        errors_.push_back(
            {.containing_folder = job.folder, .error = map_to_provider_error(contents.error)});
        return {};
    }

    for (auto item : contents.result)
    {
        std::visit(utils::overloaded{[this](entities::FileMetadata& file)
                                     { this->on_file_received(file); },
                                     [this](entities::FolderMetadata& folder)
                                     { this->queue_discovery(folder); }},
                   item);
    }

    return {};
}

void ConcurrentFileDiscovery::on_file_received(entities::FileMetadata& file)
{
    std::unique_lock<std::mutex> lock{*this->file_mutex_};
    this->gathered_files_.push_back(file);
}

void ConcurrentFileDiscovery::on_files_received(std::vector<entities::FileMetadata>& files)
{
    std::unique_lock<std::mutex> lock{*this->file_mutex_};
    gathered_files_.insert(gathered_files_.end(), files.begin(), files.end());
}

void ConcurrentFileDiscovery::queue_discovery(entities::FolderMetadata folder)
{
    this->queue_job({.folder = std::move(folder)}, [](std::monostate _) {});
}

FileDiscoveryResult ConcurrentFileDiscovery::wait_for_all_files()
{
    this->wait_for_all_queued();
    return FileDiscoveryResult{
        .files = gathered_files_,
        .errors = errors_,
    };
}
} // namespace file_discovery
