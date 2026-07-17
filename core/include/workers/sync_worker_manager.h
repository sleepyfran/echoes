#pragma once

#include "entities/provider.h"
#include "workers/sync_worker.h"
#include <map>

namespace sync
{
/**
 * A manager that can hold all the initialized providers that have associated active workers. The
 * manager should be the central state and orchestrator between all different sync workers.
 *
 * The worker manager is **not** thread safe and should **not** be used from multiple threads at
 * once. It's merely meant to be the API for workers to be used from the main UI thread.
 */
class SyncWorkerManager
{
    std::map<entities::ProviderId, SyncWorker> active_worker_map;

  public:
    /**
     * Starts the given provider if it's been correctly initialized (i.e., it's been previously
     * authorized and configured). If the provider is already running or in a non-stopped state,
     * this is a no-op.
     */
    void start_provider(entities::StartArgs);

    /**
     * Stops the given provider if it was previously started and it's in a non-stopped state,
     * otherwise this is a no-op.
     */
    void stop_provider(entities::ProviderId);

    /**
     * Subscribes to the events raised by the worker. The worker then will call the given callback
     * with the event as it happens.
     *
     * If the provider hasn't been previously initialized, it creates a new one to track.
     */
    [[nodiscard]] std::optional<SubscriptionId>
        subscribe(entities::ProviderId, std::function<void(entities::SyncWorkerEvent)>);

    /**
     * Unsubscribes the given subscription from new events raised by the specified provider ID.
     */
    void unsubscribe(entities::ProviderId, SubscriptionId id);
};
} // namespace sync
