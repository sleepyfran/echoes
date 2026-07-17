#include "workers/sync_worker_manager.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "workers/sync_worker.h"

namespace sync
{
void SyncWorkerManager::start_provider(entities::StartArgs start_args)
{
    auto [it, inserted] =
        active_worker_map.try_emplace(start_args.provider_id, start_args.provider_id);
    SyncWorker& worker = it->second;
    worker.raise(entities::SyncWorkerStartMessage{.args = start_args});
}

void SyncWorkerManager::stop_provider(entities::ProviderId provider_id)
{
    auto worker_lookup = active_worker_map.find(provider_id);
    if (worker_lookup == active_worker_map.end())
    {
        return;
    }

    SyncWorker& worker = worker_lookup->second;
    worker.raise(entities::SyncWorkerStopMessage{});
}

std::optional<SubscriptionId>
SyncWorkerManager::subscribe(entities::ProviderId provider_id,
                             std::function<void(entities::SyncWorkerEvent)> on_event)
{
    auto [it, inserted] = active_worker_map.try_emplace(provider_id, provider_id);
    SyncWorker& worker = it->second;
    return worker.subscribe(std::move(on_event));
}

void SyncWorkerManager::unsubscribe(entities::ProviderId provider_id, SubscriptionId id)
{
    auto worker_lookup = active_worker_map.find(provider_id);
    if (worker_lookup == active_worker_map.end())
    {
        return;
    }

    SyncWorker& worker = worker_lookup->second;
    worker.unsubscribe(id);
}
} // namespace sync
