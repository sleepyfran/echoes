#include "handlers/args.h"
#include "handlers/login.h"
#include "unsafe_file_auth_store.h"
#include <iostream>
#include <string_view>

void print_usage(std::string_view executable_name)
{
    std::cerr << "Usage: " << executable_name << " login onedrive\n";
}

int main(int argc, char* argv[])
{
    UnsafeFileAuthStore auth_store;
    try
    {
        auth_store.load_from_file();
    }
    catch (...)
    {
        std::cerr << "Unable to create config file, auth information won't be saved";
    }

    const std::string_view executable_name = argc > 0 ? argv[0] : "echoes-cli";

    Args args{};
    for (int i = 1; i < argc; i++)
    {
        const std::string_view argument = argv[i];
        if (argument.empty())
        {
            continue;
        }

        if (argument.starts_with("--"))
        {
            args.flags.push_back(argument.substr(2));
        }
        else if (argument.starts_with("-"))
        {
            args.flags.push_back(argument.substr(1));
        }
        else
        {
            args.positional.push_back(argument);
        }
    }

    if (!args.positional.empty() && args.positional[0] == "login")
    {
        return handle_login_command(auth_store, args);
    }

    print_usage(executable_name);
    return 1;
}
