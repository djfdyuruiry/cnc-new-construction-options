#include "json.h"

#include <bitset>

void CncJsonUtils::Cstr_Field_From_Json(
    const json& j,
    const std::string_view& json_path,
    const std::string_view& field_name,
    char* field,
    const unsigned int& length
)
{
    const auto value = j.at(field_name).get<std::string>();

    if (CncStringUtils::Is_Blank(value) || value.length() >= length) {
        CNC_LOGGER_ERROR(
            "Invalid {}{} JSON value - expected a non-blank string with 1-{} characters, actual value: {}",
            json_path,
            field_name,
            length - 1,
            value
        );

        return;
    }

    value.copy(field, length);
}
