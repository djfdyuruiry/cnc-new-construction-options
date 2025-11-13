#pragma once

#include <optional>

#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// emulate C# style nameof
#define NAMEOF(SYMBOL) #SYMBOL

// to_json macros
#define FIELD_TO_JSON(FIELD) j.emplace(#FIELD, p.FIELD)
#define FIELD_TO_JSON_WITH_TYPE(FIELD, TYPE) j.emplace(#FIELD, static_cast<TYPE>(p.FIELD))
#define BITFIELD_TO_JSON(FIELD) j.emplace(#FIELD, (bool)p.FIELD)
#define BITFIELD_OF_WIDTH_TO_JSON(FIELD, WIDTH) j.emplace(#FIELD, std::bitset<WIDTH>(p.FIELD).to_string())
#define CONVERT_FIELD_VALUE_TO_JSON(FIELD, CONVERTER, VALUE) j.emplace(#FIELD, CONVERTER(p.VALUE))
#define CONVERT_FIELD_TO_JSON(FIELD, CONVERTER) CONVERT_FIELD_VALUE_TO_JSON(FIELD, CONVERTER, FIELD)

// from_json macros
#define FIELD_FROM_JSON(FIELD) j.at(#FIELD).get_to(p.FIELD)
#define FIELD_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = j.at(#FIELD).get<TYPE>()
#define BITFIELD_FROM_JSON(FIELD) p.FIELD = j.at(#FIELD).get<bool>()
#define TRY_PARSE_BITFIELD_FROM_JSON(FIELD, WIDTH) j.at(#FIELD).get<std::string>().length() == WIDTH \
    ? std::optional(std::bitset<WIDTH>(j.at(#FIELD).get<std::string>())) \
    : std::nullopt

// to_json/from_json shorthand
# define JSON_FUNCTIONS(TYPE) friend void to_json(json& j, const TYPE& p); \
friend void from_json(const json& j, TYPE& p);
