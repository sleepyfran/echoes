#include "setup.h"
#include "auth_store.h"
#include "components/list_selector.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "providers/media_provider.h"
#include "providers/provider_factory.h"
#include <iostream>

namespace
{
void print_usage()
{
    std::cerr << "Usage: echoes-cli setup onedrive\n";
}

void handle_selected_item(std::optional<entities::ItemMetadata>& item)
{
    if (!item)
    {
        std::cout << "Nothing selected, skipping setup...\n";
        return;
    }

    std::cout << "You've selected \"" << entities::base_metadata(item.value()).name
              << "\". Goodbye!\n";

    // TODO: Start sync with selected folder.
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
    if (provider_id == std::nullopt)
    {
        std::cerr << "Unsupported provider: " << provider_id_str << '\n';
        return 1;
    }

    // TODO: Implement actual start flow.
    const auto file_based_provider =
        providers::create_file_based_provider(provider_id.value(), {.auth_store = &auth_store});
    if (!file_based_provider)
    {
        std::cerr << provider_id_str << " is not a file-based provider.\n";
        return 1;
    }

    std::cout << "Retrieving root folder contents...\n";
    auto res = file_based_provider.value()->list_root();
    switch (res.status)
    {
    case media_provider::MediaProviderResultStatus::Ok:
    {
        auto item = components::select_from_list(res.result, [](const auto& item)
                                                 { return entities::base_metadata(item).name; });
        handle_selected_item(item);
        break;
    }
    default:
    {
        std::cerr << "Something went wrong!\n";
        return 1;
    }
    }

    return 0;
}
