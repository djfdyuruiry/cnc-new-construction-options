#pragma once

#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include "common/enum.h"
#include "common/json.h"
#include "common/twowaymap.h"
#include "common/rulesections.h"
#include "common/stringutils.h"
#include "common/lua/luaengine.h"

#include "enumtypeinfo.h"
#include "target.h"
#include "technotypejsonreference.h"
#include "type.h"
#include "typeconvertermacros.h"
#include "typevariants.h"

#define TYPE_ID(TYPE) typeid(TYPE).hash_code()

/**
 * Implementation of TypeConverter concept found in common/rulesections.h for Tiberian Dawn.
 *
 * Uses magic_enum library for enum type reflection and TwoWayMap to convert between strings
 * and enum values, matching INI names for strings. EnumTypeInfo is used to construct the TwoWayMap
 * by excluding values and patching string representations that don't match INI strings.
 *
 * This provides reflection and conversion for all supported enum types: to/from string, get instances,
 * convert lists of values, get type names.
 *
 * Supports both compile-time access via templates and runtime access using ConverterTypeVariant.
 *
 * Has specific methods for working Getting/Setting INI values for class enum fields, and for converting class
 * enum fields and object references to/from JSON.
 *
 * Stores registry of rules that have been loaded into RuleSection instances which require conversion to/from
 * string representations (values based on TD enum values, not plain numbers/strings/booleans).
 *
 * To add a new type to the converter:
 *
 *   - Add type to TD_ENUMS_FORMAT macro (see typevariants.h)
 *   - Add entry to TdTypeConverter::EnumTypes with relevant values (see enumtypeinfo.h)
 *
 */
class TdTypeConverter final
{
public:
    static constexpr std::string_view EnumPostfix = "Type";
    static const std::unordered_map<size_t, std::string_view> TypeNamePatchTable;
    static const std::unordered_map<size_t, EnumTypeInfoVariant> EnumTypes;

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static const EnumTypeInfo<T>& Get_Info_For_Type()
    {
        if (!EnumTypes.contains(TYPE_ID(T))) {
            throw std::invalid_argument("Attempted to get info for an unsupported EnumTypeInfoVariant type, "
                                        "this is normally caused by variant being updated without updating "
                                        "supporting code");
        }

        const auto& type_info_variant = EnumTypes.at(TYPE_ID(T));
        const auto type_info_ptr = std::get_if<EnumTypeInfo<T>>(&type_info_variant);

        if (type_info_ptr == nullptr) {
            throw std::invalid_argument("Attempted to get info for an unsupported EnumTypeInfoVariant type, "
                                        "this is normally caused by variant being updated without updating "
                                        "supporting code");
        }

        return *type_info_ptr;
    }

    /**
     * Get a static TwoWayMap instance that allows translating between type T instances
     * and strings. This map is initialised once at first call to this method for type T,
     * in an atomic fashion.
     *
     * Limits, exclusions and STRING patches to match INI values are applied here - see enumtypeinfo.h
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static const TwoWayMap<T, std::string>& Get_Type_Map()
    {
        static std::unique_ptr<TwoWayMap<T, std::string>> type_map;
        static std::once_flag once_flag;

        // create type map once, the first time T is requested
        std::call_once(once_flag, [&] {
            const auto& enum_info = Get_Info_For_Type<T>();
            const auto enum_pairs = magic_enum::enum_entries<T>();

            std::vector<std::pair<T, std::string>> instance_pairs;

            // take the magic_enum pairs and add those that are included (patching name string as needed)
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

            if (!enum_info.AllowNonEnumValuesInRange) {
                type_map.reset(new TwoWayMap<T, std::string>(instance_pairs));
                return;
            }

            const auto min = static_cast<int>(enum_info.MinimumToInclude);
            const auto max = static_cast<int>(enum_info.MaximumToInclude);

            // we need to fill in the gaps between types as declared enum instances
            // (since AllowNonEnumValuesInRange is true)
            for (auto i = min; i <= max; ++i) {
                const auto instance = static_cast<T>(i);

                if (enum_info.Is_Excluded(instance)) {
                    continue;
                }

                // if there is no enum entry declared for the current value, magic_enum::enum_name returns blank
                if (!CncStringUtils::Is_Blank(magic_enum::enum_name<T>(instance))) {
                    continue;
                }

                std::pair<T, std::string> pair = { instance, std::format("{}", i) };

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
    static std::string To_String(const T& instance)
    {
        const auto& type_map = Get_Type_Map<T>();
        const auto instance_string = type_map[instance];

        if (!instance_string.has_value()) {
            CNC_LOGGER_DEBUG(
                "Attempt was made to convert an invalid {} value to string: {}",
                Get_Type_Name<T>(),
                static_cast<int>(instance)
            );
        }

        // use first value as default (either X_NONE or first valid value)
        const auto result = instance_string.has_value()
            ? *instance_string
            : To_String(type_map.First_Forward());

        if (!instance_string.has_value()) {
            CNC_LOGGER_WARN(
                "Attempt was made to convert excluded value (string='{}' | int={}) of type '{}' to string, returning default value: {}",
                magic_enum::enum_name(instance),
                static_cast<int>(instance),
                Get_Type_Name<T>(),
                result
            );
        }

        return result;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const std::vector<T>& instances)
    {
        static const std::function<std::string (const T&)> to_string = [](const T& v) { return To_String<T>(v); };

        return CncStringUtils::To_Csv(instances, to_string);
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const T* instances, const unsigned int& size)
    {
        std::vector<T> items;
        items.assign(instances, instances + size);

        return To_Csv_String(items);
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
    static T Assert_Parse(const std::string& str, const std::string& assert_message)
    {
        const auto result = TdTypeConverter::Try_Parse<T>(str);

        if (!result.has_value()) {
            CNC_LOGGER_FATAL(
                std::vformat(assert_message, std::make_format_args(str))
            );
        }

        return *result;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static T Assert_Parse_Lua_String(const SharedLuaEngine& engine, std::string instance_string)
    {
        const auto instance = Try_Parse<T>(instance_string);

        if (!instance.has_value()) {
            engine.Raise_Error_Format("Failed to parse {} from string: {}", Get_Type_Name<T>(), instance_string);
        }

        return *instance;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static T Get_Default_Value()
    {
        return Get_Type_Map<T>().First_Forward();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::optional<std::vector<T>> Try_Parse_Csv(const std::string& csv_str, const char& delimiter = ',')
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

    static void Reset_Rule_Type_Registry();

    /**
     * Record that a rule for the given type name requires a converter to read/write from.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static void Register_Rule_Type(const RuleSection& section, const std::string_view& rule)
    {
        const auto& type_name = section.Get_Converter_Section_Type_Name();

        type_name.has_value()
            ? RegisteredRuleTypes[type_name.value().data()][rule.data()] = Get_Default_Value<T>()
            : RegisteredRuleTypes[section.Get_Section_Name().data()][rule.data()] = Get_Default_Value<T>();
    }

    /**
     * Record that a rule for the given type name requires a CSV converter to read/write from.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static void Register_Csv_Rule_Type(const RuleSection& section, const std::string_view& rule)
    {
        const auto& section_name = section.Get_Converter_Section_Type_Name();

        section_name.has_value()
            ? RegisteredCsvRuleTypes[section_name.value().data()][rule.data()] = Get_Default_Value<T>()
            : RegisteredCsvRuleTypes[section.Get_Section_Name().data()][rule.data()] = Get_Default_Value<T>();
    }

    /**
     * Does the given type name rule require a converter to read/write from?
     */
    static bool Rule_Requires_Converter(
        const RuleSection& section,
        const std::string_view& rule
    );

    /**
     * Does the given type name rule require a CSV converter to read/write from?
     */
    static bool Rule_Requires_Csv_Converter(const RuleSection& section, const std::string_view& rule);

    /**
     * Get the corresponding variant for a given type rule, it must have been registered by calling
     * Rule_Requires_Converter first.
     */
    static ConverterTypeVariant Get_Rule_Variant(const RuleSection& section, const std::string_view& rule);

    /**
     * Get the corresponding variant for a given type csv rule, it must have been registered by calling
     * Rule_Requires_Csv_Converter first.
     */
    static ConverterTypeVariant Get_Csv_Rule_Variant(const RuleSection& section, const std::string_view& rule);

    /**
     * Using a given type rule variant, call RuleSection::Set_With_Converter with appropriate type arguments.
     */
    static void Set_Rule_With_Variant(
        RuleSection& section,
        const std::string_view& rule,
        const std::string& value,
        const ConverterTypeVariant& variant
    );

    /**
     * Using a given type rule variant, call RuleSection::Set_With_Csv_Converter with appropriate type arguments.
     */
    static void Set_Csv_Rule_With_Variant(
        RuleSection& section,
        const std::string_view& rule,
        const std::string& csv_value,
        const ConverterTypeVariant& variant
    );

    /**
     * Return a human-readable name for a given converter type.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string_view Get_Type_Name()
    {
        static std::string type_name;
        static std::once_flag once_flag;

        // resolve type name once, the first time T is requested
        std::call_once(once_flag, [&] {
            // get enum type name and remove EnumPostfix
            const auto raw_type_name = std::string(magic_enum::enum_type_name<T>());

            if (TypeNamePatchTable.contains(TYPE_ID(T))) {
                type_name = TypeNamePatchTable.at(TYPE_ID(T));
                return;
            }

            type_name = raw_type_name.substr(
                0,
                raw_type_name.length() - EnumPostfix.length()
            );
        });

        return type_name;
    }

    /**
     * Return a human-readable name for a given type rule variant type.
     */
    static std::string_view Get_Type_Name_Variant(const ConverterTypeVariant& variant);

    static std::string To_String_Variant(const ConverterTypeVariant& variant);

    static std::vector<std::string> Get_Valid_Strings_Variant(const ConverterTypeVariant& variant);

    static std::optional<std::string> RTTI_Instance_To_String(const RTTIType& type, const int& instance_id);

    static std::optional<int> Try_Parse_RTTI_Instance(const RTTIType& type, const std::string& instance);

    template<SupportedByTdTypeConverter T>
    static T Load_Field_From_Json(
        const nlohmann::json& source,
        const std::string_view& target,
        const std::string_view& field_name
    )
    {
        return Load_Value_From_Json<T>(source.at(field_name), target, field_name);
    }

    template<SupportedByTdTypeConverter T>
    static T Load_Value_From_Json(
        const nlohmann::json& json_value,
        const std::string_view& target,
        const std::string_view& field_name
    )
    {
        const auto json_path = std::format("{}.{}", target, field_name);

        CncJsonUtils::Assert_Json_Is<JsonString>(json_value, json_path);

        const auto json_string = json_value.get<std::string>();
        const auto parse_result = Try_Parse<T>(json_string);

        if (!parse_result.has_value()) {
            CncJsonUtils::Throw_Json_Assert_Failure(
                json_path,
                CncJsonUtils::Build_Parse_Error(
                    std::format("{} instance", Get_Type_Name<T>()),
                    json_string
                )
            );
        }

        return *parse_result;
    }

    template<SupportedByTdTypeConverter T>
    static void Load_Csv_Field_From_Json(
        const nlohmann::json& source,
        const std::string_view& target_name,
        const std::string_view& field_name,
        const unsigned int& expected_length,
        T* target
    )
    {
        const auto json_path = std::format("{}.{}", target_name, field_name);

        auto const& json_value = source.at(field_name);

        CncJsonUtils::Assert_Json_Is<JsonString>(json_path, json_path);

        const auto json_string = json_value.get<std::string>();
        const auto parse_result = Try_Parse_Csv<T>(json_string);

        if (!parse_result.has_value()) {
            CncJsonUtils::Throw_Json_Assert_Failure(
                json_path,
                CncJsonUtils::Build_Parse_Error(
                    std::format("csv string of {} instances", Get_Type_Name<T>()),
                    json_string
                )
            );
        }

        if (parse_result->size() != expected_length) {
            CncJsonUtils::Throw_Json_Assert_Failure(
                json_path,
                " expected csv of {} {} instances, actual length: {}",
                expected_length,
                Get_Type_Name<T>(),
                parse_result->size()
            );
        }

        std::ranges::copy(*parse_result, target);
    }

    static nlohmann::json Object_Target_Array_To_Json(
        const ObjectClass* const* source,
        const unsigned int& length
    );

    static nlohmann::json Techno_Type_To_Reference_Json(const ObjectTypeClass* source);

    template<class T>
    requires std::is_base_of_v<ObjectClass, T>
    static void Object_Target_Array_From_Json(
        const nlohmann::json& source,
        const std::string_view& target_name,
        const std::string_view& field_name,
        T** target,
        const unsigned int& length
    )
    {
        const auto json_path = std::format("{}.{}", target_name, field_name);

        CncJsonUtils::Assert_Json_Is_Array_Of_Exact_Size<JsonUnsignedInt>(source, json_path, length);

        for (auto i = 0; i < length; i++) {
            auto& element = *(target + i);
            element = OBJECT_TARGET_TO_PTR(source.at(i).get<TARGET>());
        }
    }

    static TechnoTypeClassJsonReference Techno_Type_Reference_From_Json(
        const nlohmann::json& source,
        const std::string& json_path
    );

    template<class T, class U>
    requires std::is_base_of_v<ObjectTypeClass, T> && SupportedByTdTypeConverter<U>
    static void Techno_Type_Target_From_Json(
        const nlohmann::json& source,
        const std::string_view& target_name,
        const std::string_view& field_name,
        T*& target
    )
    {
        const auto json_path = std::format("{}.{}", target_name, field_name);

        const auto [ kind_type, instance_str ] = Techno_Type_Reference_From_Json(source, json_path);

        if (kind_type == KIND_NONE) {
            target = nullptr;
            return;
        }

        auto parsed_instance = Try_Parse<U>(instance_str);

        if (!parsed_instance.has_value()) {
            CncJsonUtils::Throw_Json_Assert_Failure(
                std::format("{}.{}", json_path, NAMEOF(Instance)),
                CncJsonUtils::Build_Parse_Error(
                    std::format("techno type target instance of type {}", Get_Type_Name<U>()),
                    instance_str
                )
            );
        }

        target = reinterpret_cast<T*>(*parsed_instance);
    }

private:
    static inline const auto& Logger = CncLogger::For(TdTypeConverter);
    static inline std::unordered_map<std::string, std::unordered_map<std::string, ConverterTypeVariant>> RegisteredRuleTypes;
    static inline std::unordered_map<std::string, std::unordered_map<std::string, ConverterTypeVariant>> RegisteredCsvRuleTypes;

    TdTypeConverter() = delete;
};

// std::format support for TD enum types
template <SupportedByTdTypeConverter T>
struct std::formatter<T> : std::formatter<std::string> {
    auto format(T value, format_context& ctx) const {
        return formatter<string>::format(TdTypeConverter::To_String_Variant(value), ctx);
    }
};

// spdlog fmt library support for TD enum types (used by spdlog)
template <SupportedByTdTypeConverter T>
struct fmt::formatter<T> : fmt::formatter<std::string> {
    auto format(const T& value, fmt::format_context& ctx) const {
        return formatter<std::string>::format(TdTypeConverter::To_String_Variant(value), ctx);
    }
};

// std::format support for TD enum variant
template <>
struct std::formatter<ConverterTypeVariant> : std::formatter<std::string> {
    auto format(ConverterTypeVariant value, format_context& ctx) const {
        return formatter<string>::format(TdTypeConverter::To_String_Variant(value), ctx);
    }
};

// spdlog fmt library support for TD enum variant (used by spdlog)
template <>
struct fmt::formatter<ConverterTypeVariant> : fmt::formatter<std::string> {
    auto format(const ConverterTypeVariant& value, fmt::format_context& ctx) const {
        return formatter<std::string>::format(TdTypeConverter::To_String_Variant(value), ctx);
    }
};
