#pragma once

#include <bitset>
#include <functional>
#include <optional>

#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

#include "logger.h"
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
#define TARGET_PTR_TO_JSON(FIELD) FIELD_VALUE_TO_JSON(FIELD, p.FIELD == nullptr ? 0 : p.FIELD->As_Target())
#define TECHNO_TARGET_PTR_TO_JSON(FIELD) FIELD_VALUE_TO_JSON(FIELD, p.FIELD == nullptr ? 0 : p.FIELD->TechnoType_To_Target())
// TODO: Array equivs for PTR macros
// TODO: Move to TD Type converter (PTR macros)

// from_json macros
#define FIELD_FROM_JSON_TO_VALUE(FIELD, VALUE) j.at(#FIELD).get_to(VALUE)
#define FIELD_FROM_JSON(FIELD) j.at(#FIELD).get_to(p.FIELD)
#define FIELD_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = j.at(#FIELD).get<TYPE>()
#define BITFIELD_FROM_JSON(FIELD) p.FIELD = j.at(#FIELD).get<bool>()

// TODO: BUG - Change to just read TARGET value and stop (need to call Decode_Pointers after all object heaps rebuilt)
#define TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = (TYPE*)As_Object(j.at(#FIELD).get<TARGET>(), false); \
    Check_Ptr((void*)p.FIELD, __FILE__, __LINE__)
#define TARGET_PTR_FROM_JSON(FIELD) TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, ObjectClass)
#define TECHNO_TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = (TYPE*)Target_To_TechnoType(j.at(#FIELD).get<TARGET>(), false); \
    Check_Ptr((void*)p.FIELD, __FILE__, __LINE__)
#define TECHNO_TARGET_PTR_FROM_JSON(FIELD) TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, TechnoTypeClass)
// TODO: Array equivs for PTR macros
// TODO: Move to TD Type converter (PTR macros)

// to_json/from_json shorthand
# define JSON_FUNCTIONS(TYPE) friend void to_json(nlohmann::json& j, const TYPE& p); \
friend void from_json(const nlohmann::json& j, TYPE& p);
# define STRUCT_JSON_FUNCTIONS(TYPE) void to_json(nlohmann::json& j, const TYPE& p); \
void from_json(const nlohmann::json& j, TYPE& p);

#define TO_JSON(TYPE) void to_json(nlohmann::json& j, const TYPE& p)
#define FROM_JSON(TYPE) void from_json(const nlohmann::json& j, TYPE& p)

#define BASE_CLASS_TO_JSON(CLASS) to_json(j, static_cast<const CLASS&>(p))
#define BASE_CLASS_FROM_JSON(CLASS) from_json(j, static_cast<CLASS&>(p))

// static helper functions
class CncJsonUtils final
{
public:
    static void Cstr_Field_From_Json(
        const nlohmann::json& j,
        const std::string_view& json_path,
        const std::string_view& field_name,
        char* field,
        const unsigned int& length
    );

    template<int N>
    static void Bitfield_Of_Width_From_Json(
        const nlohmann::json& j,
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
#define CSTR_FIELD_FROM_JSON(CLASS, FIELD) \
    CncJsonUtils::Cstr_Field_From_Json(j, #CLASS, #FIELD, p.FIELD, std::size(p.FIELD) - 1)

#define BITFIELD_OF_WIDTH_FROM_JSON(CLASS, FIELD, WIDTH) \
    CncJsonUtils::Bitfield_Of_Width_From_Json<WIDTH>(j, #CLASS, #FIELD, [&](const auto& v) { p.FIELD = v.to_ulong(); })
