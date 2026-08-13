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
        on_error_received(job.folder, map_to_provider_error(contents.error));
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

void ConcurrentFileDiscovery::on_done()
{
    ChunkReadyCallback callback;
    std::vector<entities::FileMetadata> chunk;
    {
        std::scoped_lock lock{*this->subscription_mutex_, *this->file_mutex_};
        if (!subscription_)
        {
            return;
        }

        size_t begin_offset =
            std::min(subscription_->last_notified_chunk_idx, gathered_files_.size());

        if (begin_offset >= gathered_files_.size())
        {
            return;
        }

        chunk.assign(gathered_files_.begin() + static_cast<std::ptrdiff_t>(begin_offset),
                     gathered_files_.end());
        subscription_->last_notified_chunk_idx = gathered_files_.size();
        callback = subscription_->callback;
    }

    callback(chunk);
}

void ConcurrentFileDiscovery::maybe_notify_subscription()
{
    std::unique_lock<std::mutex> lock{*this->subscription_mutex_};
    if (!subscription_)
    {
        return;
    }

    if (gathered_files_.size() - subscription_->last_notified_chunk_idx >=
        subscription_->max_chunk_size)
    {
        auto& sub = subscription_.value();
        size_t end_idx = std::min(sub.last_notified_chunk_idx, gathered_files_.size());
        std::span<entities::FileMetadata> chunk(&gathered_files_[end_idx], sub.max_chunk_size);

        sub.callback(chunk);

        sub.last_notified_chunk_idx += subscription_->max_chunk_size;
    }
}

void ConcurrentFileDiscovery::on_error_received(const entities::FolderMetadata& folder,
                                                const entities::ProviderError& error)
{
    std::unique_lock<std::mutex> lock{*this->file_mutex_};
    errors_.push_back({.containing_folder = folder, .error = error});
}

void ConcurrentFileDiscovery::on_file_received(entities::FileMetadata& file)
{
    std::unique_lock<std::mutex> lock{*this->file_mutex_};
    gathered_files_.push_back(file);
    maybe_notify_subscription();
}

void ConcurrentFileDiscovery::on_files_received(std::vector<entities::FileMetadata>& files)
{
    std::unique_lock<std::mutex> lock{*this->file_mutex_};
    gathered_files_.insert(gathered_files_.end(), files.begin(), files.end());
    maybe_notify_subscription();
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

void ConcurrentFileDiscovery::subscribe(ChunkReadyCallback on_chunk_ready, size_t max_chunk_size)
{
    std::unique_lock<std::mutex> lock{*this->subscription_mutex_};
    subscription_ = Subscription{
        .callback = std::move(on_chunk_ready),
        .last_notified_chunk_idx = 0,
        .max_chunk_size = max_chunk_size,
    };
}

void ConcurrentFileDiscovery::unsubscribe()
{
    std::unique_lock<std::mutex> lock{*this->subscription_mutex_};
    subscription_ = std::nullopt;
}
} // namespace file_discovery
