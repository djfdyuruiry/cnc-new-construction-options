#pragma once

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "common/twowaymap.h"

#include "defines.h"

template<typename T>
concept SupportedByTdTypeConverter = (
    std::is_same_v<T, ArmorType>
);

#define ARMOR_PAIR(ARMOR_NAME) { ARMOR_##ARMOR_NAME, #ARMOR_NAME }

class TdTypeConverter final
{
public:
    inline static const TwoWayMap<ArmorType, std::string> Armor_Types {
        ARMOR_PAIR(NONE),
        ARMOR_PAIR(WOOD),
        ARMOR_PAIR(ALUMINUM),
        ARMOR_PAIR(STEEL),
        ARMOR_PAIR(CONCRETE),
    };

    template<SupportedByTdTypeConverter T>
    static std::string To_String(T& instance)
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            Armor_Types[instance].value_or(
                Armor_Types[ARMOR_NONE].value()
            );
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<SupportedByTdTypeConverter T>
    static std::optional<T> Try_Parse(std::string str)
    {
        if (str.empty() || std::all_of(str.begin(), str.end(), [](auto c) { return std::isspace(c); }))
        {
            return std::nullopt;
        }

        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[str];
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

private:
    TdTypeConverter() = delete;
};
