#pragma once

#include "entities/provider.h"
#include "entities/sync_messages.h"
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>

namespace sync
{
using SubscriptionId = uint8_t;

class SyncWorker
{
    entities::ProviderId provider_id_;
    entities::ProviderStatus status_;
    std::vector<std::function<void(entities::SyncWorkerEvent)>> subscriptions;
    std::mutex message_mutex_;
    std::deque<entities::SyncWorkerMessage> messages_;
    std::condition_variable_any message_condition_var_;
    std::jthread message_thread_;

    void notify(entities::SyncWorkerEvent) const;
    void run(const std::stop_token& stop_token);

  public:
    SyncWorker(entities::ProviderId);

    /**
     * Raises an event to the worker to be picked up as soon as the next processing tick happens.
     */
    void raise(const entities::SyncWorkerMessage&);

    /**
     * Subscribes to the events raised by the worker. The worker then will call the given callback
     * with the event as it happens.
     */
    [[nodiscard]] SubscriptionId subscribe(std::function<void(entities::SyncWorkerEvent)> on_event);

    /**
     * Unsubscribes from the given subscription ID. Non-existing subscriptions that match that ID is
     * a no-op.
     */
    void unsubscribe(SubscriptionId id);

    /**
     * Returns the current status of the worker.
     */
    [[nodiscard]] const entities::ProviderStatus& status() const;
};
}; // namespace sync
