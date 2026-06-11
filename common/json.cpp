#include "json.h"

#include <format>

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
