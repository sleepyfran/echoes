#include "workers/sync_worker_manager.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "providers/provider_factory.h"
#include "workers/sync_worker.h"
#include <stop_token>

namespace sync
{
SyncWorker& try_create_provider(WorkerMap& map, const entities::ProviderId& provider_id,
                                providers::GlobalDependencies deps)
{
    auto media_provider = providers::create_provider(provider_id, deps);
    auto [it, _] = map.try_emplace(provider_id, std::move(media_provider), std::stop_source{});
    auto& [worker, _2] = it->second;
    return worker;
}

void SyncWorkerManager::start_provider(const entities::StartArgs& start_args)
{
    auto& worker =
        try_create_provider(this->active_worker_map, start_args.provider_id, this->global_deps);
    worker.raise(entities::SyncWorkerStartMessage{.args = start_args});
}

void SyncWorkerManager::stop_provider(const entities::ProviderId& provider_id)
{
    auto worker_lookup = active_worker_map.find(provider_id);
    if (worker_lookup == active_worker_map.end())
    {
        return;
    }

    auto& [_, stop_source] = worker_lookup->second;
    stop_source.request_stop();
}

std::optional<SubscriptionId>
SyncWorkerManager::subscribe(const entities::ProviderId& provider_id,
                             std::function<void(entities::SyncWorkerEvent)> on_event)
{
    auto& worker = try_create_provider(this->active_worker_map, provider_id, this->global_deps);
    return worker.subscribe(std::move(on_event));
}

void SyncWorkerManager::unsubscribe(const entities::ProviderId& provider_id,
                                    const SubscriptionId& id)
{
    auto worker_lookup = active_worker_map.find(provider_id);
    if (worker_lookup == active_worker_map.end())
    {
        return;
    }

    auto& [worker, _] = worker_lookup->second;
    worker.unsubscribe(id);
}
} // namespace sync
