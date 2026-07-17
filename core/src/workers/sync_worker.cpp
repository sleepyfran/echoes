#include "workers/sync_worker.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "utils.h"
#include <iostream>
#include <mutex>
#include <stop_token>
#include <variant>

namespace sync
{
SyncWorker::SyncWorker(entities::ProviderId provider_id)
    : status_{entities::ProviderStatusNotStarted{}}, provider_id_{provider_id},
      message_thread_{[this](const auto& token) { run(token); }}
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

        std::visit(utils::overloaded{[this](const entities::SyncWorkerStartMessage& m)
                                     {
                                         entities::ProviderStatusSyncing next_status;
                                         notify(entities::SyncWorkerEventStatusChanged{
                                             .previous = status_,
                                             .current = next_status,
                                         });
                                         status_ = next_status;
                                     },
                                     [this](const entities::SyncWorkerStopMessage& m)
                                     {
                                         entities::ProviderStatusStopped next_status{};
                                         notify(entities::SyncWorkerEventStatusChanged{
                                             .previous = status_,
                                             .current = next_status,
                                         });
                                         status_ = next_status;
                                     },
                                     [this](const entities::SyncWorkerForceSyncMessage& m)
                                     {
                                         // TODO: Implement.
                                     }},
                   message);
    }
}

void SyncWorker::notify(entities::SyncWorkerEvent event) const
{
    for (const auto& subscription : subscriptions)
    {
        subscription(event);
    }
}

void SyncWorker::raise(const entities::SyncWorkerMessage& message)
{
    std::unique_lock<std::mutex> lock{message_mutex_};
    messages_.emplace_back(message);
    message_condition_var_.notify_one();
}

SubscriptionId SyncWorker::subscribe(std::function<void(entities::SyncWorkerEvent)> on_event)
{
    subscriptions.push_back(std::move(on_event));
    return subscriptions.size() - 1;
}

void SyncWorker::unsubscribe(SubscriptionId id)
{
    const unsigned long current_count = subscriptions.size();
    if (id < current_count && id > current_count)
    {
        return;
    }

    subscriptions.erase(subscriptions.begin() + id);
}

const entities::ProviderStatus& SyncWorker::status() const
{
    return status_;
}
} // namespace sync
