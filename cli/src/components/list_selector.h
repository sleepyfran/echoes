#pragma once

#include <iostream>
#include <optional>
#include <vector>

namespace components
{
/**
 * Displays all given items to the user using the provided `to_str` parameter to compute the
 * representation, then asks the user for a number representing the position of that item to select
 * it. If a selection is made, returns the item, otherwise returns nullopt.
 */
template <typename T, typename F>
    requires std::is_invocable_r_v<std::string, F, const T&>
std::optional<T> select_from_list(std::vector<T> items, F to_str)
{
    for (int i = 1; auto& item : items)
    {
        std::cout << (i++) << ". " << to_str(item) << "\n";
    }

    std::cout << "Enter a number to select (or a non-number to omit): ";
    std::string input;
    std::cin >> input;

    try
    {
        int index = std::stoi(input);
        if (index <= 0 && index > items.size())
        {
            return std::nullopt;
        }

        return items[index - 1];
    }
    catch (...)
    {
        return std::nullopt;
    }
}
} // namespace components
