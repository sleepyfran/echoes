#pragma once

#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "providers/media_provider.h"
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
    std::unique_ptr<media_provider::MediaProvider> provider;
    std::mutex message_mutex_;
    std::deque<entities::SyncWorkerMessage> messages_;
    std::condition_variable_any message_condition_var_;
    std::jthread message_thread_;

    void run(const std::stop_token& stop_token);
    void notify_status_changed(const entities::ProviderStatus& status);

  public:
    SyncWorker(std::unique_ptr<media_provider::MediaProvider>);

    /**
     * Raises an event to the worker to be picked up as soon as the next processing tick happens.
     */
    void raise(const entities::SyncWorkerMessage&);

    /**
     * Requests the worker to stop syncing, but keeps the worker alive to be able to receive new
     * messages. The worker will stop syncing at the next checkpoint. If the worker is not syncing,
     * this is a no-op.
     */
    void stop_sync();

    /**
     * Returns the current status of the worker.
     */
    [[nodiscard]] const entities::ProviderStatus& status() const;
};
}; // namespace sync
