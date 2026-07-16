#include "handlers/args.h"
#include "handlers/login.h"
#include "handlers/setup.h"
#include "unsafe_file_auth_store.h"
#include <iostream>
#include <span>
#include <string_view>

Args parse_args(std::span<char*> argv)
{
    Args args{};

    for (auto& arg : argv)
    {
        auto argument = std::string_view(arg);
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

    return args;
}

int handle_command(AuthStore& auth_store, const Args& args)
{
    const auto command = args.positional[0];
    if (command == "login")
    {
        return handle_login_command(auth_store, args);
    }

    if (command == "setup")
    {
        return handle_setup(auth_store, args);
    }

    return 1;
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

    // Skip the first element since we've already gathered the executable name.
    const Args args = parse_args(std::span(argv + 1, argc - 1));
    if (args.positional.empty())
    {
        return 1;
    }

    return handle_command(auth_store, args);
}
