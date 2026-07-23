#pragma once

#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "pubsub.h"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <stop_token>
#include <thread>

namespace sync
{

class SyncWorker : public PubSub<entities::SyncWorkerEvent>
{
    entities::ProviderId provider_id_;
    entities::ProviderStatus status_;
    std::mutex message_mutex_;
    std::deque<entities::SyncWorkerMessage> messages_;
    std::condition_variable_any message_condition_var_;
    std::jthread message_thread_;

    void run(const std::stop_token& stop_token);

  public:
    SyncWorker(entities::ProviderId);

    /**
     * Raises an event to the worker to be picked up as soon as the next processing tick happens.
     */
    void raise(const entities::SyncWorkerMessage&);

    /**
     * Returns the current status of the worker.
     */
    [[nodiscard]] const entities::ProviderStatus& status() const;
};
}; // namespace sync
