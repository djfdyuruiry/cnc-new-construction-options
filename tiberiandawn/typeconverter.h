#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#define MAGIC_ENUM_RANGE_MAX 256 // ensure all values for 'unsigned char' enums are detected
#include <magic_enum.hpp>

#include "common/json.h"
#include "common/twowaymap.h"
#include "common/rulesections.h"
#include "common/stringutils.h"

#include "defines.h"

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
    std::is_same_v<T, ScenarioVarType> ||
    std::is_same_v<T, SourceType> ||
    std::is_same_v<T, RadarEnum> ||
    std::is_same_v<T, RTTIType> ||
    std::is_same_v<T, ZoneType> ||
    std::is_same_v<T, StateType> ||
    std::is_same_v<T, VoxType> ||
    std::is_same_v<T, MouseType> ||
    std::is_same_v<T, TheaterType>
);

// Matches the SupportedByTdTypeConverter Concept types
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
    ScenarioVarType,
    SourceType,
    RadarEnum,
    RTTIType,
    ZoneType,
    StateType,
    VoxType,
    MouseType,
    TheaterType
>;

/**
 * Stores metadata about an enum type in Tiberian Dawn. Used to filter which values are exposed
 * in the Rule engine, Lua engine and INI rules. Patches enum names that don't match their INI
 * names (e.x. a Guard Tower is STRUCT_GTOWER, but is "GTWR" in INI files)
 */
template <SupportedByTdTypeConverter T>
class EnumTypeInfo final
{
public:
    const std::string Prefix;
    const T MinimumToInclude;
    const T MaximumToInclude;
    const TwoWayMap<T, std::string> PatchTable;
    const std::vector<T> Excluded;

    EnumTypeInfo(
        const std::string_view& prefix,
        const T& minimum_to_include,
        const T& maximum_to_include,
        const TwoWayMap<T, std::string>& patch_table = {},
        const std::vector<T>& excluded = {}
    ) : Prefix(prefix),
        MinimumToInclude(minimum_to_include),
        MaximumToInclude(maximum_to_include),
        PatchTable(patch_table),
        Excluded(excluded) {}

    std::string Strip_Prefix(const std::string& subject) const
    {
        if (!subject.starts_with(Prefix)) {
            return subject;
        }

        return subject.substr(Prefix.size());
    }

    std::string Strip_Prefix(const std::string_view& subject) const
    {
        return Strip_Prefix(std::string(subject));
    }

    bool Is_Excluded(const T& instance) const
    {
        if (instance < MinimumToInclude || instance > MaximumToInclude) {
            return true;
        }

        return std::find(Excluded.begin(), Excluded.end(), instance) != Excluded.end();
    }

    std::optional<std::string> Get_Patch_String(const T& instance) const
    {
        auto result = PatchTable[instance];

        if (!result.has_value()) {
            return std::nullopt;
        }

        return std::string(*result);
    }

    std::optional<T> Get_Patch_Instance(const std::string& subject) const
    {
        auto result = PatchTable[subject];

        if (!result.has_value()) {
            return std::nullopt;
        }

        return *result;
    }
};

// Allows template type EnumTypeInfo to be stored in stl container
using EnumTypeInfoVariant = std::variant<
    EnumTypeInfo<ArmorType>,
    EnumTypeInfo<MPHType>,
    EnumTypeInfo<WeaponType>,
    EnumTypeInfo<HousesType>,
    EnumTypeInfo<StructType>,
    EnumTypeInfo<FactoryType>,
    EnumTypeInfo<DirType>,
    EnumTypeInfo<BSizeType>,
    EnumTypeInfo<AircraftType>,
    EnumTypeInfo<MissionType>,
    EnumTypeInfo<AnimType>,
    EnumTypeInfo<InfantryType>,
    EnumTypeInfo<UnitType>,
    EnumTypeInfo<SpeedType>,
    EnumTypeInfo<BulletType>,
    EnumTypeInfo<WarheadType>,
    EnumTypeInfo<VocType>,
    EnumTypeInfo<PlayerColorType>,
    EnumTypeInfo<HouseColorType>,
    EnumTypeInfo<DiffType>,
    EnumTypeInfo<ScenarioDirType>,
    EnumTypeInfo<ScenarioVarType>,
    EnumTypeInfo<SourceType>,
    EnumTypeInfo<RadarEnum>,
    EnumTypeInfo<RTTIType>,
    EnumTypeInfo<ZoneType>,
    EnumTypeInfo<StateType>,
    EnumTypeInfo<VoxType>,
    EnumTypeInfo<MouseType>,
    EnumTypeInfo<TheaterType>
>;

/**
 * Implementation of TypeConverter concept found in common/rulesections.h for Tiberian Dawn.
 *
 * Uses magic_enum library for enum type reflection and TwoWayMap to convert between strings
 * and enum values, matching INI names for strings. EnumTypeInfo is used to construct the TwoWayMap
 * by excluding values and patching string representations that don't match INI strings.
 *
 * This provides reflection and conversion for all support enum types: to/from string, get instances,
 * convert lists of values, get type names.
 *
 * Supports both compile-time access via templates and runtime access using ConverterTypeVariant.
 *
 * Has specific methods for working Getting/Setting INI values for class enum fields, and for converting class
 * enum fields to/from JSON.
 *
 * Stores registry of rules that have been loaded into RuleSection instances which require conversion to/from
 * string representations (values based on TD enum values, not plain numbers/strings/booleans).
 *
 * To add a new type to the converter:
 *
 *   - Add type to SupportedByTdTypeConverter, ConverterTypeVariant and EnumTypeInfoVariant (above)
 *   - Add entry to TdTypeConverter::EnumTypes with relevant values
 *   - Update Variant method bodies in typeconverter.cpp to handle new types
 */
class TdTypeConverter final
{
public:
    static const inline std::string_view EnumPostfix = "Type";
    // Info about each enum type, indexed against it's typename
    static const std::map<std::string_view, EnumTypeInfoVariant> EnumTypes;

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static const EnumTypeInfo<T>& Get_Info_For_Type()
    {
        const auto type_name = Get_Type_Name<T>();

        if (!EnumTypes.contains(type_name)) {
            throw std::invalid_argument("Attempted to get info for an unsupported EnumTypeInfoVariant type, "
                                        "this is normally caused by variant being updated without updating "
                                        "supporting code");
        }

        const auto& type_info_variant = EnumTypes.at(type_name);
        const auto type_info = std::get_if<EnumTypeInfo<T>>(&type_info_variant);

        if (type_info == nullptr) {
            throw std::invalid_argument("Attempted to get info for an unsupported EnumTypeInfoVariant type, "
                                        "this is normally caused by variant being updated without updating "
                                        "supporting code");
        }

        return *type_info;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static const TwoWayMap<T, std::string>& Get_Type_Map()
    {
        static std::shared_ptr<TwoWayMap<T, std::string>> type_map;
        static std::once_flag onceFlag;

        // create type map once, the first time it's requested
        std::call_once(onceFlag, [&] {
            const auto& enum_info = Get_Info_For_Type<T>();
            const auto enum_pairs = magic_enum::enum_entries<T>();

            std::vector<std::pair<T, std::string>> instance_pairs;

            for (const auto& [instance, instance_string] : enum_pairs) {
                if (enum_info.Is_Excluded(instance)) {
                    continue;
                }

                auto patch_string = enum_info.Get_Patch_String(instance);
                auto ini_string = patch_string.has_value()
                    ? *patch_string
                    : enum_info.Strip_Prefix(instance_string);

                std::pair<T, std::string> pair = { instance, ini_string };

                instance_pairs.emplace_back(pair);
            }

            type_map.reset(new TwoWayMap<T, std::string>(instance_pairs));
        });

        return *type_map;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<std::string> Get_Valid_Strings()
    {
        return Get_Type_Map<T>().Get_Backward_Keys();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<T> Get_Valid_Instances()
    {
        return Get_Type_Map<T>().Get_Forward_Keys();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_String(T instance)
    {
        const auto& type_map = Get_Type_Map<T>();
        const auto instance_string = type_map[instance];

        if (!instance_string.has_value()) {
            CNC_LOGGER_WARN(
                "Attempt was made to convert an invalid {} value to string: {}",
                Get_Type_Name<T>(),
                static_cast<int>(instance)
            );
        }

        // use first value as default (either X_NONE or first valid value)
        return instance_string.value_or(
            type_map.First_Backward()
        );
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

        auto result = Get_Type_Map<T>()[str];

        return result;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static T Get_Default_Value()
    {
        return Get_Type_Map<T>().First_Forward();
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
        // get enum type name and remove EnumPostfix
        static const auto raw_type_name = std::string(magic_enum::enum_type_name<T>());
        static const auto type_name_without_prefix = raw_type_name.substr(
            0,
            raw_type_name.length() - EnumPostfix.length()
        );

        CNC_LOG_INFO(type_name_without_prefix);

        return type_name_without_prefix;
    }

    /**
     * Return a human-readable name for a given type rule variant type.
     */
    static std::string_view Get_Type_Name_Variant(ConverterTypeVariant variant);

    // TODO: Ability to set default (if source doesn't contain the field)
    template<SupportedByTdTypeConverter T>
    static bool Load_Field_From_Json(
        const json& source,
        std::string_view json_path,
        std::string_view field_name,
        std::function<void(T)> with_valid_value
    )
    {
        if (!source.contains(field_name)) {
            // TODO: Log
            return false;
        }

        auto const& json_value = source.at(field_name);

        if (!json_value.is_string()) {
            // TODO: Log
            return false;
        }

        const auto json_string = json_value.get<std::string>();
        const auto parse_result = Try_Parse<T>(json_string);

        if (!parse_result.has_value()) {
            CNC_LOGGER_ERROR(
                "Invalid {} JSON value - expected {} instance, actual value: {}",
                json_path,
                field_name,
                Get_Type_Name<T>(),
                json_string
            );

            return false;
        }

        with_valid_value(*parse_result);

        return true;
    }
private:
    static inline const auto& Logger = CncLogger::For(TdTypeConverter);
    static inline std::map<std::string_view, std::map<std::string_view, ConverterTypeVariant>> RegisteredRuleTypes;
    static inline std::map<std::string_view, std::map<std::string_view, ConverterTypeVariant>> RegisteredCsvRuleTypes;

    TdTypeConverter() = delete;
};

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Read_With_TdConverter(TYPE, VAR) \
    Get_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](const auto& v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Read_Csv_With_TdConverter(TYPE, VAR) \
    Get_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](auto v) { VAR = std::move(v); })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](const auto& v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Load_Csv_With_TdConverter(TYPE, VAR) \
    Load_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = std::move(v); })

// JSON macros

// Convert TD type field to string and store in JSON object, actual field value can be any expression (e.g. fetch Type enum value from pointer object)
#define CONVERT_TD_FIELD_VALUE_TO_JSON(FIELD, VALUE) \
    CONVERT_FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::To_String, VALUE)

// Convert TD type field to string and store in JSON object
#define CONVERT_TD_FIELD_TO_JSON(FIELD) CONVERT_TD_FIELD_VALUE_TO_JSON(FIELD, FIELD)

// Parse TD type field from JSON string
#define PARSE_TD_FIELD_FROM_JSON(CLASS, FIELD, TYPE) \
    TdTypeConverter::Load_Field_From_Json<TYPE>(j, #CLASS, #FIELD, [&](const auto& v) { p.FIELD = v; })

#define TD_TYPE_POINTER_FROM_JSON(CLASS, FIELD, TYPE, TYPECLASS) TdTypeConverter::Load_Field_From_Json<TYPE>( \
    j, \
    #CLASS, \
    #FIELD, \
    [&](const auto& h) { \
        const_cast<TYPECLASS const*&>(p.FIELD) = &TYPECLASS::As_Reference(h); \
    } \
)
