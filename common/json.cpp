#include "json.h"

#include <format>

#include <magic_enum/magic_enum.hpp>

#include "stringutils.h"

std::string CncJsonUtils::Build_Type_Error(const std::string& expected_type, const nlohmann::json& subject)
{
    return std::format("expected {}, actual type: {}", expected_type, subject.type_name());
}

std::string CncJsonUtils::Build_Parse_Error(
    const std::string& attempted_parse_type,
    const std::string& subject,
    const std::string_view& error
)
{
    return std::format(
        "unable to parse as {}, json value: {} | error = {}",
        attempted_parse_type,
        subject,
        error
    );
}


void CncJsonUtils::Assert_Json_Is_Object_With_Keys(
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

    if (missing_keys.empty()) {
        return;
    }

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

void CncJsonUtils::Assert_Json_Is_Object_With_Keys_And_Types(
    const nlohmann::json& subject,
    const std::string& json_path,
    const std::vector<std::string>& expected_keys,
    const std::unordered_map<std::string, nlohmann::json::value_t>& expected_types
)
{
    Assert_Json_Is_Object_With_Keys(subject, json_path, expected_keys);

    std::unordered_map<std::string, nlohmann::json::value_t> invalid_types;

    for (const auto& [ key, expected_type ] : expected_types) {
        if (expected_type == NotNullJsonType && !subject[key].is_null()) {
            // custom not null check
            continue;
        }

        const auto actual_type = subject[key].type();

        if (actual_type != expected_type) {
            invalid_types[key] = actual_type;
        }
    }

    if (invalid_types.empty()) {
        return;
    }

    // use magic_enum to lookup names for json type enum
    static constexpr auto json_types_enums = magic_enum::enum_entries<nlohmann::json::value_t>();
    static const auto json_type_names = std::unordered_map(json_types_enums.begin(), json_types_enums.end());

    std::vector<std::string> invalid_keys;

    for (const auto& [ key, invalid_type ] : invalid_types) {
        const auto& expected_type = expected_types.at(key);
        const auto expected_type_name = expected_type == NotNullJsonType
            ? "not null"
            : json_type_names.at(expected_type);
        const auto actual_type_name = json_type_names.at(invalid_type);

        invalid_keys.emplace_back(
            std::format(
                "expected '.{}' to be of type '{}' - actual type: {}",
                key,
                expected_type_name,
                actual_type_name
            )
        );
    }

    Throw_Json_Assert_Failure(
        json_path,
        "expected object with correct key types: {}",
        CncStringUtils::To_Csv(invalid_keys)
    );
}

void CncJsonUtils::Cstr_Field_From_Json(
    const nlohmann::json& j,
    const std::string_view& json_path,
    const std::string_view& field_name,
    char* field,
    const unsigned int& str_length
)
{
    const auto sub_path = std::format("{}{}", json_path, field_name);
    const auto& field_json = j[field_name];

    Assert_Json_Is<JsonString>(field_json, sub_path);

    const auto value = field_json.get<std::string>();

    if (value.length() > str_length) {
        Throw_Json_Assert_Failure(
            sub_path,
            Build_Parse_Error(
                std::format("string with at most {} characters", str_length),
                value,
                "string too long"
            )
        );
    }

    const auto copied_length = value.copy(field, str_length);
    field[copied_length] = '\0';
}