#include "json.h"

#include <bitset>

void CncJsonUtils::Cstr_Field_From_Json(
    const nlohmann::json& j,
    const std::string_view& json_path,
    const std::string_view& field_name,
    char* field,
    const unsigned int& str_length
)
{
    const auto& field_json = j.at(field_name);

    if (!field_json.is_string()) {
        throw CncJsonException(
            "Invalid {}{} JSON value - expected a string, actual type: {}",
            json_path,
            field_name,
            field_json.type_name()
        );
    }

    const auto value = field_json.get<std::string>();

    if (value.length() > str_length) {
        throw CncJsonException(
            "Invalid {}{} JSON value - expected a string with at most {} characters, actual value: {}",
            json_path,
            field_name,
            str_length,
            value
        );
    }

    const auto copied_length = value.copy(field, str_length);
    field[copied_length] = '\0';
}

std::string CncJsonUtils::Build_Type_Error(const std::string& expected_type, const nlohmann::json& subject)
{
    return std::format("expected {}, actual type: {}", expected_type, subject.type_name());
}
