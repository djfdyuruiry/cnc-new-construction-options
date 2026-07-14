#pragma once

#include <format>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <fmt/format.h>

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

#define TD_TYPE_NAME(TYPE) TdTypeConverter::Get_Type_Names().at(typeid(TYPE).hash_code())
#define TD_TYPE_MAP(TYPE) std::get<TwoWayMap<TYPE>>( \
    TdTypeConverter::Get_Type_Maps().at(typeid(TYPE).hash_code()) \
)

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
 *   - Update TD_ENUMS_FORMAT macro definition to include the new type (see typevariants.h)
 *   - Add entry to TdTypeConverter::EnumTypes with relevant values (see enumtypeinfo.h)
 *
 */
class TdTypeConverter final
{
public:
    static constexpr std::string_view EnumPostfix = "Type";
    static const std::unordered_map<std::string, std::string_view> TypeNamePatchTable;
    static const std::unordered_map<std::string_view, EnumTypeInfoVariant> EnumTypes;

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static const EnumTypeInfo<T>& Get_Info_For_Type()
    {
        const auto type_name = TD_TYPE_NAME(T);

        if (!EnumTypes.contains(type_name)) {
            throw std::invalid_argument("Attempted to get info for an unsupported EnumTypeInfoVariant type, "
                                        "this is normally caused by variant being updated without updating "
                                        "supporting code");
        }

        const auto& type_info_variant = EnumTypes.at(type_name);
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
    static const TwoWayMap<T>& Get_Type_Map()
    {
        return TD_TYPE_MAP(T);
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<std::string> Get_Valid_Strings()
    {
        return TD_TYPE_MAP(T).Get_Backward_Keys();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::vector<T> Get_Valid_Instances()
    {
        return TD_TYPE_MAP(T).Get_Forward_Keys();
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static std::string To_Csv_String(const std::vector<T>& instances)
    {
        static const std::function<std::string (const T&)> to_string = [](const T& v) { return To_String(v); };

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

        auto result = TD_TYPE_MAP(T)[str];

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
            engine.Raise_Error_Format("Failed to parse {} from string: {}", TD_TYPE_NAME(T), instance_string);
        }

        return *instance;
    }

    template<class T>
    requires SupportedByTdTypeConverter<T>
    static T Get_Default_Value()
    {
        return TD_TYPE_MAP(T).First_Forward();
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
    static void Register_Rule_Type(const std::string_view& type_name, const std::string_view& rule)
    {
        RegisteredRuleTypes[type_name.data()][rule.data()] = Get_Default_Value<T>();
    }

    /**
     * Record that a rule for the given type name requires a CSV converter to read/write from.
     */
    template<class T>
    requires SupportedByTdTypeConverter<T>
    static void Register_Csv_Rule_Type(const std::string_view& type_name, const std::string_view& rule)
    {
        RegisteredCsvRuleTypes[type_name.data()][rule.data()] = Get_Default_Value<T>();
    }

    /**
     * Does the given type name rule require a converter to read/write from?
     */
    static bool Rule_Requires_Converter(const std::string_view& type_name, const std::string_view& rule);

    /**
     * Overload for Rule_Requires_Converter to allow providing source RuleSection directly.
     */
    static bool Rule_Requires_Converter(
        const RuleSection& section,
        const std::string_view& rule
    );

    /**
     * Does the given type name rule require a CSV converter to read/write from?
     */
    static bool Rule_Requires_Csv_Converter(const std::string_view& type_name, const std::string_view& rule);

    /**
     * Overload for Rule_Requires_Csv_Converter to allow providing source RuleSection directly.
     */
    static bool Rule_Requires_Csv_Converter(
        const RuleSection& section,
        const std::string_view& rule
    );

    /**
     * Get the corresponding variant for a given type rule, it must have been registered by calling
     * Rule_Requires_Converter first.
     */
    static ConverterTypeVariant Get_Rule_Variant(const std::string_view& type_name, const std::string_view& rule);

    /**
     * Get the corresponding variant for a given type csv rule, it must have been registered by calling
     * Rule_Requires_Csv_Converter first.
     */
    static ConverterTypeVariant Get_Csv_Rule_Variant(const std::string_view& type_name, const std::string_view& rule);

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
        return TD_TYPE_NAME(T);
    }

    /**
     * Return a human-readable name for a given type rule variant type.
     */
    static std::string_view Get_Type_Name_Variant(const ConverterTypeVariant& variant);

    static std::string To_String(const ConverterTypeVariant& variant);

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
                    std::format("{} instance", TD_TYPE_NAME(T)),
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
                    std::format("csv string of {} instances", TD_TYPE_NAME(T)),
                    json_string
                )
            );
        }

        if (parse_result->size() != expected_length) {
            CncJsonUtils::Throw_Json_Assert_Failure(
                json_path,
                " expected csv of {} {} instances, actual length: {}",
                expected_length,
                TD_TYPE_NAME(T),
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
                    std::format("techno type target instance of type {}", TD_TYPE_NAME(U)),
                    instance_str
                )
            );
        }

        target = reinterpret_cast<T*>(*parsed_instance);
    }

private:
    static inline const auto& Logger = CncLogger::For(TdTypeConverter);

    static inline std::unordered_map<size_t, std::string_view> TypeNames;
    static inline std::unordered_map<size_t, EnumTwoWayMapVariant> TypeMaps;

    static inline std::unordered_map<std::string, std::unordered_map<std::string, ConverterTypeVariant>> RegisteredRuleTypes;
    static inline std::unordered_map<std::string, std::unordered_map<std::string, ConverterTypeVariant>> RegisteredCsvRuleTypes;

    static const std::unordered_map<size_t, std::string_view>& Get_Type_Names();
    static const std::unordered_map<size_t, EnumTwoWayMapVariant>& Get_Type_Maps();

    TdTypeConverter() = delete;
};

// std::format support for TD enum types
template <SupportedByTdTypeConverter T>
struct std::formatter<T> : std::formatter<std::string> {
    auto format(T value, format_context& ctx) const {
        return formatter<string>::format(TdTypeConverter::To_String(value), ctx);
    }
};

// spdlog fmt library support for TD enum types (used by spdlog)
template <SupportedByTdTypeConverter T>
struct fmt::formatter<T> : fmt::formatter<std::string> {
    auto format(const T& value, fmt::format_context& ctx) const {
        return formatter<std::string>::format(TdTypeConverter::To_String(value), ctx);
    }
};

// std::format support for TD enum variant
template <>
struct std::formatter<ConverterTypeVariant> : std::formatter<std::string> {
    auto format(ConverterTypeVariant value, format_context& ctx) const {
        return formatter<string>::format(TdTypeConverter::To_String(value), ctx);
    }
};

// spdlog fmt library support for TD enum variant (used by spdlog)
template <>
struct fmt::formatter<ConverterTypeVariant> : fmt::formatter<std::string> {
    auto format(const ConverterTypeVariant& value, fmt::format_context& ctx) const {
        return formatter<std::string>::format(TdTypeConverter::To_String(value), ctx);
    }
};
