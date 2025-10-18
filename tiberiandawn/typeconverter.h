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
    std::is_same_v<T, ArmorType> ||
    std::is_same_v<T, MPHType> ||
    std::is_same_v<T, WeaponType>
);

#define ARMOR_PAIR(ARMOR_NAME) { ARMOR_##ARMOR_NAME, #ARMOR_NAME }
#define MPH_PAIR(MPH_NAME) { MPH_##MPH_NAME, #MPH_NAME }
#define WEAPON_PAIR(WEAPON_NAME) { WEAPON_##WEAPON_NAME, #WEAPON_NAME }

class TdTypeConverter final
{
public:
    inline static const TwoWayMap<ArmorType, std::string> Armor_Types {
        ARMOR_PAIR(NONE),
        ARMOR_PAIR(WOOD),
        ARMOR_PAIR(ALUMINUM),
        ARMOR_PAIR(STEEL),
        ARMOR_PAIR(CONCRETE)
    };
    inline static const TwoWayMap<MPHType, std::string> Mph_Types {
        MPH_PAIR(IMMOBILE),
        MPH_PAIR(VERY_SLOW),
        MPH_PAIR(KINDA_SLOW),
        MPH_PAIR(SLOW),
        MPH_PAIR(SLOW_ISH),
        MPH_PAIR(MEDIUM_SLOW),
        MPH_PAIR(MEDIUM),
        MPH_PAIR(MEDIUM_FAST),
        MPH_PAIR(MEDIUM_FASTER),
        MPH_PAIR(FAST),
        MPH_PAIR(ROCKET),
        MPH_PAIR(VERY_FAST),
        MPH_PAIR(LIGHT_SPEED)
    };
    inline static const TwoWayMap<WeaponType, std::string> Weapon_Types {
        WEAPON_PAIR(NONE),
        WEAPON_PAIR(RIFLE),
        WEAPON_PAIR(CHAIN_GUN),
        WEAPON_PAIR(PISTOL),
        WEAPON_PAIR(M16),
        WEAPON_PAIR(DRAGON),
        WEAPON_PAIR(FLAMETHROWER),
        WEAPON_PAIR(FLAME_TONGUE),
        WEAPON_PAIR(CHEMSPRAY),
        WEAPON_PAIR(GRENADE),
        WEAPON_PAIR(75MM),
        WEAPON_PAIR(105MM),
        WEAPON_PAIR(120MM),
        WEAPON_PAIR(TURRET_GUN),
        WEAPON_PAIR(MAMMOTH_TUSK),
        WEAPON_PAIR(MLRS),
        WEAPON_PAIR(155MM),
        WEAPON_PAIR(M60MG),
        WEAPON_PAIR(TOMAHAWK),
        WEAPON_PAIR(TOW_TWO),
        WEAPON_PAIR(NAPALM),
        WEAPON_PAIR(OBELISK_LASER),
        WEAPON_PAIR(NIKE),
        WEAPON_PAIR(HONEST_JOHN),
        WEAPON_PAIR(STEG),
        WEAPON_PAIR(TREX)
    };

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_String(T instance)
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[instance].value_or(
                Armor_Types[ARMOR_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types[instance].value_or(
                Mph_Types[MPH_IMMOBILE].value()
            );
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types[instance].value_or(
                Weapon_Types[WEAPON_NONE].value()
            );
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::optional<T> Try_Parse(std::string str)
    {
        if (str.empty() || std::all_of(str.begin(), str.end(), [](auto c) { return std::isspace(c); }))
        {
            return std::nullopt;
        }

        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[str];
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types[str];
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types[str];
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

private:
    TdTypeConverter() = delete;

};

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = v; })
