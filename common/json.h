#pragma once

#include <bitset>
#include <functional>
#include <optional>

#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

#include "logger.h"
#include "stringutils.h"

using json = nlohmann::json;

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
#define TARGET_TO_JSON(FIELD) FIELD_VALUE_TO_JSON(FIELD, TARGET_SAFE_CAST(p.FIELD))

// from_json macros
#define FIELD_FROM_JSON(FIELD) j.at(#FIELD).get_to(p.FIELD)
#define FIELD_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = j.at(#FIELD).get<TYPE>()
#define BITFIELD_FROM_JSON(FIELD) p.FIELD = j.at(#FIELD).get<bool>()
#define TARGET_FROM_JSON(FIELD, TYPE) p.FIELD = (TYPE*)(j.at(#FIELD).get<TARGET>())

// to_json/from_json shorthand
# define JSON_FUNCTIONS(TYPE) friend void to_json(json& j, const TYPE& p); \
friend void from_json(const json& j, TYPE& p);

#define TO_JSON(TYPE) void to_json(json& j, const TYPE& p)
#define FROM_JSON(TYPE) void from_json(const json& j, TYPE& p)

#define BASE_CLASS_TO_JSON(CLASS) to_json(j, static_cast<const CLASS&>(p))
#define BASE_CLASS_FROM_JSON(CLASS) from_json(j, static_cast<CLASS&>(p))

// static helper functions
class CncJsonUtils final
{
public:
    static void Cstr_Field_From_Json(
        const json& j,
        const std::string_view& json_path,
        const std::string_view& field_name,
        char* field,
        const unsigned int& length
    );

    template<int N>
    static void Bitfield_Of_Width_From_Json(
        const json& j,
        const std::string_view& json_path,
        const std::string_view& field_name,
        const std::function<void(std::bitset<N>)>& on_valid_value
    )
    {
        const auto value = j.at(field_name).get<std::string>();

        try {
            on_valid_value(std::bitset<N>(value));
        } catch (const std::out_of_range& e) {
            CNC_LOGGER_ERROR(
                "Invalid {}{} JSON value - expected {} bit binary string, actual value: {} | parse error: {}",
                json_path,
                field_name,
                N,
                value,
                e.what()
            );
        }
    }

private:
    static inline const auto& Logger = CncLogger::For(CncJsonUtils);

    CncJsonUtils() = delete;
};

// Load the value for a c-string from JSON string (with validation)
#define CSTR_FIELD_FROM_JSON(CLASS, FIELD, LENGTH) \
    CncJsonUtils::Cstr_Field_From_Json(j, #CLASS, #FIELD, p.FIELD, LENGTH)

#define BITFIELD_OF_WIDTH_FROM_JSON(CLASS, FIELD, WIDTH) \
    CncJsonUtils::Bitfield_Of_Width_From_Json<WIDTH>(j, #CLASS, #FIELD, [&](const auto& v) { p.FIELD = v.to_ulong(); })
