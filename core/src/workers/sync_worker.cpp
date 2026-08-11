#include "workers/sync_worker.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "utils.h"
#include "workers/downloader.h"
#include "workers/file_sync_worker.h"
#include <iostream>
#include <mutex>
#include <stop_token>
#include <variant>

namespace sync
{
SyncWorker::SyncWorker(std::unique_ptr<media_provider::MediaProvider> provider)
    : provider{std::move(provider)}, message_thread_{[this](const auto& token) { run(token); }}
{
}

void SyncWorker::run(const std::stop_token& token)
{
    std::stop_callback callback(token,
                                []
                                {
                                    // TODO: Handle.
                                    std::cout << "Stop requested! \n";
                                });

    while (!token.stop_requested())
    {
        entities::SyncWorkerMessage message{};
        {
            std::unique_lock<std::mutex> lock{message_mutex_};

            bool should_continue =
                message_condition_var_.wait(lock, token, [this] { return !messages_.empty(); });
            if (!should_continue)
            {
                break;
            }

            message = messages_.front();
            messages_.pop_front();
        }

        std::visit(
            utils::overloaded{
                [this, &token](const entities::SyncWorkerStartMessage& m)
                {
                    notify_status_changed(entities::ProviderStatusSyncing());

                    if (auto* file_based_provider =
                            dynamic_cast<media_provider::FileBasedProvider*>(this->provider.get()))
                    {
                        downloader::Downloader downloader{8};
                        auto args = std::get<entities::FileBasedProviderStartArgs>(m.args.args);
                        file_sync::sync_file_based_provider(this->publisher(), downloader,
                                                            args.selected_folder,
                                                            *file_based_provider, token);
                    }
                },
                [this](const entities::SyncWorkerForceSyncMessage& m)
                {
                    // TODO: Implement.
                }},
            message);
    }
}

void SyncWorker::notify_status_changed(const entities::ProviderStatus& status)
{
    publish(entities::SyncWorkerEventStatusChanged{
        .previous = this->provider->status,
        .current = status,
    });
    this->provider->status = status;
}

void SyncWorker::raise(const entities::SyncWorkerMessage& message)
{
    std::unique_lock<std::mutex> lock{message_mutex_};
    messages_.emplace_back(message);
    message_condition_var_.notify_one();
}

void SyncWorker::stop_sync()
{
    std::unique_lock<std::mutex> lock{message_mutex_};
    this->message_thread_.request_stop();
    notify_status_changed(entities::ProviderStatusStopped());
}

const entities::ProviderStatus& SyncWorker::status() const
{
    return this->provider->status;
}
} // namespace sync
