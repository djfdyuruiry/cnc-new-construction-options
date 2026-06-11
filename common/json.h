#pragma once

#include <bitset>
#include <functional>

// force manual handling of enum values - ensures CNC enum types are never serialized by value (use strings instead)
#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

#include "stringutils.h"

// emulate C# style nameof
#define NAMEOF(SYMBOL) #SYMBOL

// to_json macros
#define FIELD_VALUE_TO_JSON(FIELD, VALUE) j.emplace(#FIELD, VALUE)
#define FIELD_TO_JSON(FIELD) j.emplace(#FIELD, p.FIELD)
#define CSTR_FIELD_TO_JSON(FIELD) j.emplace(#FIELD, std::string(p.FIELD))
#define FIELD_TO_JSON_WITH_TYPE(FIELD, TYPE) j.emplace(#FIELD, static_cast<TYPE>(p.FIELD))
#define BITFIELD_TO_JSON(FIELD) j.emplace(#FIELD, (bool)p.FIELD)
#define BITFIELD_OF_WIDTH_TO_JSON(FIELD, WIDTH) j.emplace(#FIELD, std::bitset<WIDTH>(p.FIELD).to_string())
#define CONVERT_FIELD_VALUE_TO_JSON(FIELD, CONVERTER, VALUE) j.emplace(#FIELD, CONVERTER(VALUE))
#define CONVERT_FIELD_TO_JSON(FIELD, CONVERTER) j.emplace(#FIELD, CONVERTER(p.FIELD))

// from_json macros
#define FIELD_FROM_JSON_TO_VALUE(FIELD, VALUE) j[#FIELD].get_to(VALUE)
#define FIELD_FROM_JSON(FIELD) j[#FIELD].get_to(p.FIELD)
#define FIELD_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = j[#FIELD].get<TYPE>()
#define BITFIELD_FROM_JSON(FIELD) p.FIELD = j[#FIELD].get<bool>()

// to_json/from_json friend functions shorthand (reference types)
# define JSON_FUNCTIONS(TYPE) friend void to_json(nlohmann::json& j, const TYPE& p); \
    friend void from_json(const nlohmann::json& j, TYPE& p);

// to_json/from_json friend functions shorthand (pointer types)
# define JSON_PTR_FUNCTIONS(TYPE) friend void to_json(nlohmann::json& j, const TYPE* const& p); \
    friend void from_json(const nlohmann::json& j, TYPE*& p);

// to_json/from_json global functions shorthand (reference types)
# define GLOBAL_JSON_FUNCTIONS(TYPE) void to_json(nlohmann::json& j, const TYPE& p); \
    void from_json(const nlohmann::json& j, TYPE& p);

// to_json/from_json global functions shorthand (reference types)
# define GLOBAL_JSON_PTR_FUNCTIONS(TYPE) void to_json(nlohmann::json& j, const TYPE* const& p); \
    void from_json(const nlohmann::json& j, TYPE*& p);

#define TO_JSON(TYPE) void to_json(nlohmann::json& j, const TYPE& p)
#define FROM_JSON(TYPE) void from_json(const nlohmann::json& j, TYPE& p)

#define PTR_TO_JSON(TYPE) void to_json(nlohmann::json& j, const TYPE* const& p)
#define PTR_FROM_JSON(TYPE) void from_json(const nlohmann::json& j, TYPE*& p)

#define BASE_CLASS_TO_JSON(CLASS) to_json(j, static_cast<const CLASS&>(p))
#define BASE_CLASS_FROM_JSON(CLASS) from_json(j, static_cast<CLASS&>(p))

/**
 * Wrapper around runtime_error, constructor allows formatting the error message using std::vformat.
 */
class CncJsonException final : public std::runtime_error
{
public:
    template<typename... Args>
    CncJsonException(const std::string& fmt, Args&&... args)
        : runtime_error(std::vformat(fmt, std::make_format_args(args...))) {}
};

typedef nlohmann::json::string_t JsonString;
typedef nlohmann::json::boolean_t JsonBoolean;
typedef nlohmann::json::number_integer_t JsonInt;
typedef nlohmann::json::number_unsigned_t JsonUnsignedInt;
typedef nlohmann::json::number_float_t JsonFloat;
typedef nlohmann::json::array_t JsonArray;
typedef nlohmann::json::object_t JsonObject;
typedef void* JsonAny;

template <class T>
concept JsonType = std::is_same_v<T, JsonString>
        || std::is_same_v<T, JsonBoolean>
        || std::is_same_v<T, JsonInt>
        || std::is_same_v<T, JsonUnsignedInt>
        || std::is_same_v<T, JsonFloat>
        || std::is_same_v<T, JsonArray>
        || std::is_same_v<T, JsonObject>
        || std::is_same_v<T, JsonAny>;

// static helper functions
class CncJsonUtils final
{
public:
    static std::string Build_Type_Error(const std::string& expected_type, const nlohmann::json& subject);

    static std::string Build_Parse_Error(
        const std::string& attempted_parse_type,
        const std::string& subject,
        const std::string_view& error = "invalid string value"
    );

    template<typename... Args>
    static void Throw_Json_Assert_Failure(const std::string& json_path, const std::string& fmt, Args&&... args)
    {
        throw CncJsonException(
            "Invalid JSON value {} - {}",
            json_path,
            std::vformat(fmt, std::make_format_args(args...))
        );
    }

    template<JsonType T>
    static void Assert_Json_Is(const nlohmann::json& subject, const std::string& json_path)
    {
        auto assert_failed = false;
        std::string expected_type;

        if constexpr (std::is_same_v<T, JsonAny>) {
            if (assert_failed = subject.is_null(); assert_failed) {
                expected_type = "not null";
            }
        }

        if constexpr (std::is_same_v<T, JsonArray>) {
            if (assert_failed = !subject.is_array(); assert_failed) {
                expected_type = "an array";
            }
        }

        if constexpr (std::is_same_v<T, JsonObject>) {
            if (assert_failed = !subject.is_object(); assert_failed) {
                expected_type = "object";
            }
        }

        if constexpr (std::is_same_v<T, JsonUnsignedInt>) {
            if (assert_failed = !subject.is_number_unsigned(); assert_failed) {
                expected_type = "non-negative int";
            }
        }

        if constexpr (std::is_same_v<T, JsonInt>) {
            if (assert_failed = !subject.is_number_integer(); assert_failed) {
                expected_type = "int";
            }
        }

        if constexpr (std::is_same_v<T, JsonBoolean>) {
            if (assert_failed = !subject.is_boolean(); assert_failed) {
                expected_type = "bool";
            }
        }

        if constexpr (std::is_same_v<T, JsonString>) {
            if (assert_failed = !subject.is_string(); assert_failed) {
                expected_type = "string";
            }
        }

        if (assert_failed) {
            Throw_Json_Assert_Failure(json_path, Build_Type_Error(expected_type, subject.type_name()));
        }
    }

    template<JsonType T>
    static void Assert_Json_Is_Array_With_Size(
        const nlohmann::json& subject,
        const std::string& json_path,
        const unsigned int& min_size,
        std::optional<unsigned int> max_size = std::nullopt
    )
    {
        Assert_Json_Is<JsonArray>(subject, json_path);

        const auto size = subject.size();

        if (max_size.has_value()) {
            if (size < min_size || size > *max_size) {
                Throw_Json_Assert_Failure(
                    json_path,
                    "expected array with a size of {}-{} element(s), actual size: {}",
                    min_size,
                    *max_size,
                    subject.size()
                );
            }
        } else if (size < min_size) {
            Throw_Json_Assert_Failure(
                json_path,
                "expected array with a size of at least {} element(s), actual size: {}",
                min_size,
                subject.size()
            );
        }

        if (size < 1) {
            // unable to assert element type
            return;
        }

        Assert_Json_Is<T>(subject[0], std::format("{}[0]", json_path));
    }

    template<JsonType T>
    static void Assert_Json_Is_Array_Of_Exact_Size(
        const nlohmann::json& subject,
        const std::string& json_path,
        unsigned int size
    )
    {
        return Assert_Json_Is_Array_With_Size<T>(subject, json_path, size, size);
    }

    static void Assert_Json_Is_Object_With_Keys(
        const nlohmann::json& subject,
        const std::string& json_path,
        const std::vector<std::string>& expected_keys
    )
    {
        Assert_Json_Is<JsonObject>(subject, json_path);

        std::vector<std::string> missing_keys;

        for (const auto& key : expected_keys) {
            if (!subject.contains(key)) {
                missing_keys.push_back(key);
            }
        }

        if (!missing_keys.empty()) {
            std::vector<std::string> actual_keys;

            for (const auto& [key, _] : subject.items()) {
                actual_keys.emplace_back(key);
            }

            Throw_Json_Assert_Failure(
                json_path,
                "expected object with keys '{}', actual keys: {}",
                CncStringUtils::To_Csv(expected_keys),
                CncStringUtils::To_Csv(actual_keys)
            );
        }
    }

    static void Cstr_Field_From_Json(
        const nlohmann::json& j,
        const std::string_view& json_path,
        const std::string_view& field_name,
        char* field,
        const unsigned int& str_length
    );

    template<int N>
    static std::bitset<N> Bitset_Of_Width_From_Json(
        const nlohmann::json& j,
        const std::string_view& json_path,
        const std::string_view& field_name
    )
    {
        const auto& field = j[field_name];

        Assert_Json_Is<JsonString>(json_path, field);

        const auto value = field.get<std::string>();
        std::string error_message;

        try {
            return std::bitset<N>(value);
        } catch (const std::out_of_range& e) {
            error_message = e.what();
        } catch (const std::invalid_argument& e) {
            error_message = e.what();
        }

        Throw_Json_Assert_Failure(
            std::format("{}{}", json_path, field_name),
            Build_Parse_Error(
                std::format("{} bit bitset", N),
                value,
                error_message
            )
        );

        throw std::runtime_error("Throw_Json_Assert_Failure will throw before I am called");
    }

private:

    CncJsonUtils() = delete;
};

// Load the value for a c-string from JSON string (with validation)
#define CSTR_FIELD_FROM_JSON(CLASS, FIELD) \
    CncJsonUtils::Cstr_Field_From_Json(j, #CLASS, #FIELD, p.FIELD, std::size(p.FIELD) - 1)

#define BITFIELD_OF_WIDTH_FROM_JSON(CLASS, FIELD, WIDTH) \
    p.FIELD = (CncJsonUtils::Bitset_Of_Width_From_Json<WIDTH>(j, #CLASS, #FIELD)).to_ulong()
