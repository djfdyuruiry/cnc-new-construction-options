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
    const auto value = j.at(field_name).get<std::string>();

    if (CncStringUtils::Is_Blank(value) || value.length() > str_length) {
        CNC_LOGGER_ERROR(
            "Invalid {}{} JSON value - expected a non-blank string with 1-{} characters, actual value: {}",
            json_path,
            field_name,
            str_length,
            value
        );

        return;
    }

    const auto copied_length = value.copy(field, str_length);
    field[copied_length] = '\0';
}
