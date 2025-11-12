#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "common/twowaymap.h"
#include "common/stringutils.h"

#include "defines.h"
#include "rulesections.h"

using ConverterTypeVariant = std::variant<
    ArmorType,
    MPHType,
    WeaponType,
    HousesType,
    StructType,
    FactoryType,
    DirType,
    BSizeType,
    AircraftType,
    MissionType,
    AnimType,
    InfantryType,
    UnitType,
    SpeedType,
    BulletType,
    WarheadType,
    VocType,
    PlayerColorType,
    HouseColorType,
    DiffType,
    ScenarioDirType,
    ScenarioVarType
>;

template<typename T>
concept SupportedByTdTypeConverter = (
    std::is_same_v<T, ArmorType> ||
    std::is_same_v<T, MPHType> ||
    std::is_same_v<T, WeaponType> ||
    std::is_same_v<T, HousesType> ||
    std::is_same_v<T, StructType> ||
    std::is_same_v<T, FactoryType> ||
    std::is_same_v<T, DirType> ||
    std::is_same_v<T, BSizeType> ||
    std::is_same_v<T, AircraftType> ||
    std::is_same_v<T, MissionType> ||
    std::is_same_v<T, AnimType> ||
    std::is_same_v<T, InfantryType> ||
    std::is_same_v<T, UnitType> ||
    std::is_same_v<T, SpeedType> ||
    std::is_same_v<T, BulletType> ||
    std::is_same_v<T, WarheadType> ||
    std::is_same_v<T, VocType> ||
    std::is_same_v<T, PlayerColorType> ||
    std::is_same_v<T, HouseColorType> ||
    std::is_same_v<T, DiffType> ||
    std::is_same_v<T, ScenarioDirType> ||
    std::is_same_v<T, ScenarioVarType>
);

class TdTypeConverter final
{
public:
    static const TwoWayMap<ArmorType, std::string> Armor_Types;
    static const TwoWayMap<MPHType, std::string> Mph_Types;
    static const TwoWayMap<WeaponType, std::string> Weapon_Types;
    static const TwoWayMap<HousesType, std::string> House_Types;
    static const TwoWayMap<StructType, std::string> Struct_Types;
    static const TwoWayMap<FactoryType, std::string> Factory_Types;
    static const TwoWayMap<DirType, std::string> Dir_Types;
    static const TwoWayMap<BSizeType, std::string> BSize_Types;
    static const TwoWayMap<AircraftType, std::string> Aircraft_Types;
    static const TwoWayMap<MissionType, std::string> Mission_Types;
    static const TwoWayMap<AnimType, std::string> Anim_Types;
    static const TwoWayMap<InfantryType, std::string> Infantry_Types;
    static const TwoWayMap<UnitType, std::string> Unit_Types;
    static const TwoWayMap<SpeedType, std::string> Speed_Types;
    static const TwoWayMap<BulletType, std::string> Bullet_Types;
    static const TwoWayMap<WarheadType, std::string> Warhead_Types;
    // VOC_BUILD_SELECT is omitted as it appears unused an is a pointer to another VOC (non-unique)
    static const TwoWayMap<VocType, std::string> Voc_Types;
    static const TwoWayMap<PlayerColorType, std::string> Player_Color_Types;
    static const TwoWayMap<HouseColorType, std::string> House_Color_Types;
    static const TwoWayMap<DiffType, std::string> Diff_Types;
    static const TwoWayMap<ScenarioDirType, std::string> Scenario_Dir_Types;
    static const TwoWayMap<ScenarioVarType, std::string> Scenario_Var_Types;

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<std::string> Get_Valid_Strings()
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return Diff_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return Scenario_Dir_Types.Get_Backward_Keys();
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return Scenario_Var_Types.Get_Backward_Keys();
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<T> Get_Valid_Instances()
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return Diff_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return Scenario_Dir_Types.Get_Forward_Keys();
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return Scenario_Var_Types.Get_Forward_Keys();
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

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
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types[instance].value_or(
                Factory_Types[FACTORY_TYPE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types[instance].value_or(
                Dir_Types[DIR_N].value()
            );
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types[instance].value_or(
                BSize_Types[BSIZE_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types[instance].value_or(
                Aircraft_Types[AIRCRAFT_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types[instance].value_or(
                Mission_Types[MISSION_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types[instance].value_or(
                Anim_Types[ANIM_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types[instance].value_or(
                Infantry_Types[INFANTRY_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types[instance].value_or(
                Unit_Types[UNIT_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types[instance].value_or(
                Speed_Types[SPEED_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types[instance].value_or(
                Bullet_Types[BULLET_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types[instance].value_or(
                Warhead_Types[WARHEAD_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types[instance].value_or(
                Voc_Types[VOC_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types[instance].value_or(
                Player_Color_Types[REMAP_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types[instance].value_or(
                House_Color_Types[HOUSE_COLOR_GOOD].value()
            );
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return Diff_Types[instance].value_or(
                Diff_Types[DIFF_NORMAL].value()
            );
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return Scenario_Dir_Types[instance].value_or(
                Scenario_Dir_Types[SCEN_DIR_NONE].value()
            );
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return Scenario_Var_Types[instance].value_or(
                Scenario_Var_Types[SCEN_VAR_NONE].value()
            );
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const std::vector<T>& instances)
    {
        std::function<std::string (T)> to_string = [](T v) { return To_String<T>(v); };

        return CncStringUtils::To_Csv(instances, to_string);
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::optional<T> Try_Parse(std::string str)
    {
        if (CncStringUtils::Is_Blank(str))
        {
            return std::nullopt;
        }

        // forgive bad casing for type instance
        CncStringUtils::To_Upper(str);

        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types[str];
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types[str];
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types[str];
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types[str];
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types[str];
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types[str];
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types[str];
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types[str];
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types[str];
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types[str];
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types[str];
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types[str];
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types[str];
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types[str];
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types[str];
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types[str];
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types[str];
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types[str];
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types[str];
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return Diff_Types[str];
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return Scenario_Dir_Types[str];
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return Scenario_Var_Types[str];
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static T Get_Default_Value()
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return Armor_Types.First_Forward();
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return Mph_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return Weapon_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return House_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, StructType>) {
            return Struct_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return Factory_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, DirType>) {
            return Dir_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return BSize_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return Aircraft_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return Mission_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return Anim_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return Infantry_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return Unit_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return Speed_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return Bullet_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return Warhead_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, VocType>) {
            return Voc_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return Player_Color_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return House_Color_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return Diff_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return Scenario_Dir_Types.First_Forward();;
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return Scenario_Var_Types.First_Forward();;
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::optional<std::vector<T>> Try_Parse_Csv(const std::string& csv_str, const char delimiter = ',')
    {
        std::vector<T> instances;
        size_t start = 0;
        size_t end = csv_str.find(delimiter);

        while (end != std::string::npos) {
            auto entry = csv_str.substr(start, end - start);
            auto instance = Try_Parse<T>(entry);

            if (!instance.has_value()) {
                CNC_LOGGER_ERROR("Value '{}' could not be parsed as type: {}", entry, typeid(T).name());
                return std::nullopt;
            }

            instances.push_back(instance.value());

            start = end + 1;
            end = csv_str.find(delimiter, start);
        }

        auto entry = csv_str.substr(start);
        auto instance = Try_Parse<T>(entry);

        if (!instance.has_value()) {
            CNC_LOGGER_ERROR("Value '{}' could not be parsed as type: {}", entry, typeid(T).name());
            return std::nullopt;
        }

        instances.push_back(instance.value());

        return std::make_optional(instances);
    }

    /**
     * Record that a rule for the given type name requires a converter to read/write from.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static void Register_Rule_Type(std::string_view type_name, std::string_view rule)
    {
        if (!RegisteredRuleTypes.contains(type_name)) {
            RegisteredRuleTypes[type_name] = {};
        }

        RegisteredRuleTypes[type_name][rule] = Get_Default_Value<T>();
    }

    /**
     * Record that a rule for the given type name requires a CSV converter to read/write from.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static void Register_Csv_Rule_Type(std::string_view type_name, std::string_view rule)
    {
        if (!RegisteredCsvRuleTypes.contains(type_name)) {
            RegisteredCsvRuleTypes[type_name] = {};
        }

        RegisteredCsvRuleTypes[type_name][rule] = Get_Default_Value<T>();
    }

    /**
     * Does the given type name rule require a converter to read/write from?
     */
    static bool Rule_Requires_Converter(std::string_view type_name, std::string_view rule);

    /**
     * Does the given type name rule require a CSV converter to read/write from?
     */
    static bool Rule_Requires_Csv_Converter(std::string_view type_name, std::string_view rule);

    /**
     * Get the corresponding variant for a given type rule, it must have been registered by calling
     * Rule_Requires_Converter first.
     */
    static ConverterTypeVariant Get_Rule_Variant(std::string_view type_name, std::string_view rule);

    /**
     * Get the corresponding variant for a given type csv rule, it must have been registered by calling
     * Rule_Requires_Csv_Converter first.
     */
    static ConverterTypeVariant Get_Csv_Rule_Variant(std::string_view type_name, std::string_view rule);

    /**
     * Using a given type rule variant, call RuleSection::Set_With_Converter with appropriate type arguments.
     */
    static void Set_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string value, const ConverterTypeVariant variant);

    /**
     * Using a given type rule variant, call RuleSection::Set_With_Csv_Converter with appropriate type arguments.
     */
    static void Set_Csv_Rule_With_Variant(RuleSection& section, std::string_view rule, std::string csv_value, const ConverterTypeVariant variant);

    /**
     * Return a human-readable name for a given converter type.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string_view Get_Type_Name()
    {
        if constexpr (std::is_same_v<T, ArmorType>) {
            return "Armor";
        } else if constexpr (std::is_same_v<T, MPHType>) {
            return "MphSpeed";
        } else if constexpr (std::is_same_v<T, WeaponType>) {
            return "Weapon";
        } else if constexpr (std::is_same_v<T, HousesType>) {
            return "House";
        } else if constexpr (std::is_same_v<T, StructType>) {
            return "Building";
        } else if constexpr (std::is_same_v<T, FactoryType>) {
            return "Factory";
        } else if constexpr (std::is_same_v<T, DirType>) {
            return "Direction";
        } else if constexpr (std::is_same_v<T, BSizeType>) {
            return "BuildingSize";
        } else if constexpr (std::is_same_v<T, AircraftType>) {
            return "Aircraft";
        } else if constexpr (std::is_same_v<T, MissionType>) {
            return "Mission";
        } else if constexpr (std::is_same_v<T, AnimType>) {
            return "Animation";
        } else if constexpr (std::is_same_v<T, InfantryType>) {
            return "Infantry";
        } else if constexpr (std::is_same_v<T, UnitType>) {
            return "Unit";
        } else if constexpr (std::is_same_v<T, SpeedType>) {
            return "Speed";
        } else if constexpr (std::is_same_v<T, BulletType>) {
            return "Bullet";
        } else if constexpr (std::is_same_v<T, WarheadType>) {
            return "Warhead";
        } else if constexpr (std::is_same_v<T, VocType>) {
            return "SoundEffect";
        } else if constexpr (std::is_same_v<T, PlayerColorType>) {
            return "PlayerColor";
        } else if constexpr (std::is_same_v<T, HouseColorType>) {
            return "HouseColor";
        } else if constexpr (std::is_same_v<T, DiffType>) {
            return "Difficulty";
        } else if constexpr (std::is_same_v<T, ScenarioDirType>) {
            return "ScenarioDirection";
        } else if constexpr (std::is_same_v<T, ScenarioVarType>) {
            return "ScenarioVariation";
        }

        throw std::invalid_argument("Unsupported SupportedByTdTypeConverter type - this is normally caused by concept being updated without updating supporting code");
    }

    /**
     * Return a human-readable name for a given type rule variant type.
     */
    static std::string_view Get_Type_Name_Variant(ConverterTypeVariant variant);

private:
    static inline const auto& Logger = CncLogger::For(TdTypeConverter);
    static inline std::map<std::string_view, std::map<std::string_view, ConverterTypeVariant>> RegisteredRuleTypes;
    static inline std::map<std::string_view, std::map<std::string_view, ConverterTypeVariant>> RegisteredCsvRuleTypes;

    TdTypeConverter() = delete;
};

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Read_With_TdConverter(TYPE, VAR) \
    Get_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](auto v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Read_Csv_With_TdConverter(TYPE, VAR) \
    Get_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](auto v) { VAR = std::move(v); })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Load_Csv_With_TdConverter(TYPE, VAR) \
    Load_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = std::move(v); })
