#include "setup.h"
#include "auth_store.h"
#include "components/list_selector.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "providers/media_provider.h"
#include "providers/provider_factory.h"
#include "utils.h"
#include "workers/sync_worker_manager.h"
#include <future>
#include <iostream>
#include <variant>

namespace
{
void print_usage()
{
    std::cerr << "Usage: echoes-cli setup onedrive\n";
}

void on_status_change_event_received(std::promise<bool>& completion_promise,
                                     entities::SyncWorkerEventStatusChanged event)
{
    std::visit(
        utils::overloaded{
            [&completion_promise](entities::ProviderStatusNotStarted s) {},
            [&completion_promise](entities::ProviderStatusErrored s)
            {
                std::cout << "Provider has failed\n";
                completion_promise.set_value(true);
            },
            [&completion_promise](entities::ProviderStatusStopped s)
            {
                std::cout << "Provider has stopped\n";
                completion_promise.set_value(true);
            },
            [&completion_promise](entities::ProviderStatusSyncSkipped s)
            {
                std::cout << "Provider has skipped syncing\n";
                completion_promise.set_value(true);
            },
            [&completion_promise](entities::ProviderStatusSyncing s)
            { std::cout << "Provider is syncing\n"; },
            [&completion_promise](entities::ProviderStatusSynced s)
            {
                std::cout << "Provider has finished syncing " << s.synced_tracks
                          << " tracks have been correctly synced and " << s.errored_tracks
                          << " have failed " << "\n";
                completion_promise.set_value(true);
            },
        },
        event.current);
}

void handle_selected_item(providers::GlobalDependencies deps, entities::ProviderId provider_id,
                          std::optional<entities::ItemMetadata>& item)
{
    if (!item || std::holds_alternative<entities::FileMetadata>(item.value()))
    {
        std::cout << "Nothing selected, skipping setup...\n";
        return;
    }

    std::cout << "You've selected \"" << entities::base_metadata(item.value()).name << "\"\n";
    std::cout << "Starting provider...\n";

    // TODO: Move this out of the method.
    sync::SyncWorkerManager manager{deps};

    std::promise<bool> completion_promise;
    auto completion_future = completion_promise.get_future();
    auto id = manager.subscribe(
        provider_id,
        [&provider_id, &completion_promise](entities::SyncWorkerEvent event)
        {
            std::visit(
                utils::overloaded{
                    [&completion_promise](entities::SyncWorkerEventStatusChanged event)
                    { on_status_change_event_received(completion_promise, event); },
                    [](entities::SyncWorkerFolderSkippedDuringSync& event)
                    {
                        std::cout << "The following folder was ignored due to an error: "
                                  << event.folder.name << "\n";
                    },
                    [](entities::SyncWorkerFileSkippedDuringSync& event)
                    {
                        std::cout << "The following file was ignored due to an error: "
                                  << event.file.name << "\n";
                    },
                    [](entities::SyncWorkerFileProcessedDuringSync& event)
                    { std::cout << event.file.name << " processed" << "\n"; }},
                event);
        });

    if (!id)
    {
        std::cout << "Failed to subscribe\n";
        return;
    }

    entities::FileBasedProviderStartArgs args{.selected_folder =
                                                  std::get<entities::FolderMetadata>(item.value())};
    manager.start_provider({.provider_id = provider_id, .args = args});

    completion_future.get();
    std::cout << "Past the completion future\n";

    manager.unsubscribe(provider_id, id.value());
}
} // namespace

int handle_setup(AuthStore& auth_store, const Args& args)
{
    if (!args.flags.empty() || args.positional.size() != 2)
    {
        print_usage();
        return 1;
    }

    auto provider_id_str = args.positional[1];
    auto provider_id = entities::provider_from_string(provider_id_str);
    if (!provider_id)
    {
        std::cerr << "Unsupported provider: " << provider_id_str << '\n';
        return 1;
    }

    providers::GlobalDependencies deps{.auth_store = &auth_store};

    // TODO: Encapsulate this better or move somewhere else.
    auto provider = providers::create_provider(provider_id.value(), deps);
    auto* file_based_provider = dynamic_cast<media_provider::FileBasedProvider*>(provider.get());
    if (!file_based_provider)
    {
        std::cerr << provider_id_str << " is not a file-based provider.\n";
        return 1;
    }

    std::cout << "Retrieving root folder contents...\n";
    auto res = file_based_provider->list_root();
    switch (res.status)
    {
    case media_provider::MediaProviderResultStatus::Ok:
    {
        auto item = components::select_from_list(res.result, [](const auto& item)
                                                 { return entities::base_metadata(item).name; });
        handle_selected_item(deps, provider_id.value(), item);
        break;
    }
    default:
    {
        std::cerr << "Something went wrong! Error: " << static_cast<int>(res.error) << '\n';
        return 1;
    }
    }

    return 0;
}
