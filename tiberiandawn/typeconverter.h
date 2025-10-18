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
    std::is_same_v<T, WeaponType> ||
    std::is_same_v<T, HousesType> ||
    std::is_same_v<T, StructType>
);

#define ARMOR_PAIR(ARMOR_NAME) { ARMOR_##ARMOR_NAME, #ARMOR_NAME }
#define MPH_PAIR(MPH_NAME) { MPH_##MPH_NAME, #MPH_NAME }
#define WEAPON_PAIR(WEAPON_NAME) { WEAPON_##WEAPON_NAME, #WEAPON_NAME }
#define HOUSE_PAIR(HOUSE_NAME) { HOUSE_##HOUSE_NAME, #HOUSE_NAME }
#define STRUCT_PAIR(STRUCT_NAME) { STRUCT_##STRUCT_NAME, #STRUCT_NAME }

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
    inline static const TwoWayMap<HousesType, std::string> House_Types {
        HOUSE_PAIR(NONE),
        HOUSE_PAIR(GOOD),
        HOUSE_PAIR(BAD),
        HOUSE_PAIR(NEUTRAL),
        HOUSE_PAIR(JP),
        HOUSE_PAIR(MULTI1),
        HOUSE_PAIR(MULTI2),
        HOUSE_PAIR(MULTI3),
        HOUSE_PAIR(MULTI4),
        HOUSE_PAIR(MULTI5),
        HOUSE_PAIR(MULTI6)
    };
    inline static const TwoWayMap<StructType, std::string> Struct_Types {
        STRUCT_PAIR(NONE),
        STRUCT_PAIR(WEAP),
        STRUCT_PAIR(GTOWER),
        STRUCT_PAIR(ATOWER),
        STRUCT_PAIR(OBELISK),
        STRUCT_PAIR(RADAR),
        STRUCT_PAIR(TURRET),
        STRUCT_PAIR(CONST),
        STRUCT_PAIR(REFINERY),
        STRUCT_PAIR(STORAGE),
        STRUCT_PAIR(HELIPAD),
        STRUCT_PAIR(SAM),
        STRUCT_PAIR(AIRSTRIP),
        STRUCT_PAIR(POWER),
        STRUCT_PAIR(ADVANCED_POWER),
        STRUCT_PAIR(HOSPITAL),
        STRUCT_PAIR(BARRACKS),
        STRUCT_PAIR(TANKER),
        STRUCT_PAIR(REPAIR),
        STRUCT_PAIR(BIO_LAB),
        STRUCT_PAIR(HAND),
        STRUCT_PAIR(TEMPLE),
        STRUCT_PAIR(EYE),
        STRUCT_PAIR(MISSION),
        STRUCT_PAIR(V01),
        STRUCT_PAIR(V02),
        STRUCT_PAIR(V03),
        STRUCT_PAIR(V04),
        STRUCT_PAIR(V05),
        STRUCT_PAIR(V06),
        STRUCT_PAIR(V07),
        STRUCT_PAIR(V08),
        STRUCT_PAIR(V09),
        STRUCT_PAIR(V10),
        STRUCT_PAIR(V11),
        STRUCT_PAIR(V12),
        STRUCT_PAIR(V13),
        STRUCT_PAIR(V14),
        STRUCT_PAIR(V15),
        STRUCT_PAIR(V16),
        STRUCT_PAIR(V17),
        STRUCT_PAIR(V18),
        STRUCT_PAIR(PUMP),
        STRUCT_PAIR(V20),
        STRUCT_PAIR(V21),
        STRUCT_PAIR(V22),
        STRUCT_PAIR(V23),
        STRUCT_PAIR(V24),
        STRUCT_PAIR(V25),
        STRUCT_PAIR(V26),
        STRUCT_PAIR(V27),
        STRUCT_PAIR(V28),
        STRUCT_PAIR(V29),
        STRUCT_PAIR(V30),
        STRUCT_PAIR(V31),
        STRUCT_PAIR(V32),
        STRUCT_PAIR(V33),
        STRUCT_PAIR(V34),
        STRUCT_PAIR(V35),
        STRUCT_PAIR(V36),
        STRUCT_PAIR(V37),
        STRUCT_PAIR(SANDBAG_WALL),
        STRUCT_PAIR(CYCLONE_WALL),
        STRUCT_PAIR(BRICK_WALL),
        STRUCT_PAIR(BARBWIRE_WALL),
        STRUCT_PAIR(WOOD_WALL)
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
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types[instance].value_or(
                House_Types[HOUSE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types[instance].value_or(
                Struct_Types[STRUCT_NONE].value()
            );
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const std::vector<T>& instances)
    {
        std::ostringstream oss;
        auto first = true;

        for (const auto& instance : instances) {
            if (!first) {
                oss << ",";
            }

            oss << To_String<T>(instance);
            first = false;
        }

        return oss.str();
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
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types[str];
        }else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types[str];
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<T> Try_Parse_Csv(const std::string& csv_str, const char delimiter = ',')
    {
        std::vector<T> instances;
        size_t start = 0;
        size_t end = csv_str.find(delimiter);

        while (end != std::string::npos) {
            if (auto instance = Try_Parse<T>(csv_str.substr(start, end - start)); instance.has_value()) {
                instances.push_back(instance.value());
            }

            start = end + 1;
            end = csv_str.find(delimiter, start);
        }

        if (auto instance = Try_Parse<T>(csv_str.substr(start)); instance.has_value()) {
            instances.push_back(instance.value());
        }

        return instances;
    }

private:
    TdTypeConverter() = delete;

};

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Load_Csv_With_TdConverter(TYPE, VAR) \
    Load_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = std::move(v); })
