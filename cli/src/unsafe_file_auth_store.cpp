#include "unsafe_file_auth_store.h"
#include "entities/auth.h"
#include "entities/provider.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <variant>

namespace fs = std::filesystem;

fs::path get_config_directory()
{
#if defined(_WIN32)
    // Windows: %APPDATA%.
    auto* env_var = std::getenv("APPDATA");
    if (env_var)
    {
        return {env_var};
    }
#elif defined(__APPLE__)
    // macOS: ~/Library/Application Support.
    const char* home = std::getenv("HOME");
    if (home)
    {
        return fs::path(home) / "Library" / "Application Support";
    }
#else
    // Linux/Unix: XDG_CONFIG_HOME or ~/.config.
    auto* env_var = std::getenv("XDG_CONFIG_HOME");
    if (env_var)
    {
        return {env_var};
    }

    const char* home = std::getenv("HOME");
    if (home)
    {
        return fs::path(home) / ".config";
    }
#endif

    // Fallback to current directory if we cannot determine it.
    return fs::current_path();
}

fs::path create_or_get_app_directory()
{
    auto app_directory = get_config_directory() / "echoes";
    if (!fs::exists(app_directory))
    {
        fs::create_directory(app_directory);
    }

    return app_directory;
}

// TODO: Move to shared util
inline std::string provider_id_to_string(entities::ProviderId provider_id)
{
    switch (provider_id)
    {
    case entities::ProviderId::OneDrive:
        return "onedrive";
    case entities::ProviderId::Spotify:
        return "spotify";
    }

    return "";
}

fs::path unsafe_auth_store_file(entities::ProviderId provider_id)
{
    auto base_folder = create_or_get_app_directory();
    auto p_id = provider_id_to_string(provider_id);
    return base_folder / (p_id + "_auth_store");
}

void save_string(std::ofstream& stream, std::string_view str)
{
    size_t str_length = str.size();
    stream.write(reinterpret_cast<const char*>(&str_length), sizeof(str_length));
    stream.write(str.data(), str_length);
}

void save_long(std::ofstream& stream, long l)
{
    stream.write(reinterpret_cast<const char*>(&l), sizeof(l));
}

void save_provider(const entities::AuthInfo& info)
{
    auto path = unsafe_auth_store_file(info.provider_id);
    std::ofstream os(path, std::ios::binary);

    save_string(os, info.access_token);
    save_string(os, info.refresh_token);
    save_long(os, info.expires_on);

    std::visit(
        [&](const auto& provider_info)
        {
            using T = std::decay_t<decltype(provider_info)>;

            if constexpr (std::is_same_v<T, entities::MsalSpecificAuthInfo>)
            {
                save_string(os, provider_info.home_account_id);
                save_string(os, provider_info.environment);
                save_string(os, provider_info.tenant_id);
                save_string(os, provider_info.username);
                save_string(os, provider_info.local_account_id);
            }
            else if constexpr (std::is_same_v<T, entities::SpotifySpecificAuthInfo>)
            {
            }
        },
        info.provider_specific_auth_info);
}

std::string load_string(std::ifstream& stream)
{
    size_t str_length = 0;
    if (!stream.read(reinterpret_cast<char*>(&str_length), sizeof(str_length)))
    {
        return "";
    }

    std::string str(str_length, '\0');
    stream.read(str.data(), str_length);
    return str;
}

long load_long(std::ifstream& stream)
{
    long l = 0;
    stream.read(reinterpret_cast<char*>(&l), sizeof(l));
    return l;
}

std::optional<entities::AuthInfo> load_from_file_for_provider(entities::ProviderId provider_id)
{
    fs::path path = unsafe_auth_store_file(provider_id);

    if (!fs::exists(path))
    {
        return std::nullopt;
    }

    std::ifstream is(path, std::ios::binary);
    if (!is)
    {
        return std::nullopt;
    }

    entities::AuthInfo info;
    info.provider_id = provider_id;

    info.access_token = load_string(is);
    info.refresh_token = load_string(is);
    info.expires_on = load_long(is);

    if (provider_id == entities::ProviderId::OneDrive)
    {
        entities::MsalSpecificAuthInfo msal;
        msal.home_account_id = load_string(is);
        msal.environment = load_string(is);
        msal.tenant_id = load_string(is);
        msal.username = load_string(is);
        msal.local_account_id = load_string(is);

        info.provider_specific_auth_info = msal;
    }
    else if (provider_id == entities::ProviderId::Spotify)
    {
        entities::SpotifySpecificAuthInfo spotify;
        info.provider_specific_auth_info = spotify;
    }

    return info;
}

void UnsafeFileAuthStore::load_from_file()
{
    for (auto provider_id : entities::AllProviderIds)
    {
        auto data = load_from_file_for_provider(provider_id);
        if (data.has_value())
        {
            auth_info_by_provider.insert_or_assign(provider_id, data.value());
        }
    }
}

bool UnsafeFileAuthStore::save(const entities::AuthInfo& info) const
{
    const std::lock_guard lock(mutex);
    auth_info_by_provider.insert_or_assign(info.provider_id, info);
    save_provider(info);
    return true;
}

std::optional<entities::AuthInfo>
UnsafeFileAuthStore::retrieve(entities::ProviderId provider_id) const
{
    const std::lock_guard lock(mutex);
    const auto found = auth_info_by_provider.find(provider_id);
    if (found == auth_info_by_provider.end())
    {
        return std::nullopt;
    }

    return found->second;
}
