#include <ranges>

#include "rulesections.h"

TO_JSON(RuleValueVariant)
{
    j["type"]= RuleSection::Get_Variant_Type(p);

    if (const auto value = std::get_if<int>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<bool>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<float>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<ushort>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<std::string>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<uint>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<char>(&p)) {
        j["value"] = *value;
    } else if (const auto value = std::get_if<uchar>(&p)) {
        j["value"] = *value;
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }
}

FROM_JSON(RuleValueVariant)
{
    if (
        !j.is_object() ||
        !j.contains("type") ||
        !j.at("type").is_string() ||
        !j.contains("value") ||
        j.at("value").is_null()
    ) {
        throw CncJsonException(
            "Invalid {} JSON value - expected object with keys 'type' (string) and 'value' (non-null value), "
            "actual JSON: {}",
            NAMEOF(RuleValueVariant),
            j.dump()
        );
    }

    const auto variant_type = j.at("type").get<std::string>();
    auto& variant_value = j.at("value");

    if (variant_type == "int") {
        p = variant_value.get<int>();
    } else if (variant_type == "bool") {
        p = variant_value.get<bool>();
    } else if (variant_type == "float") {
        p = variant_value.get<float>();
    } else if (variant_type == "unsigned short") {
        p = variant_value.get<ushort>();
    } else if (variant_type == "string") {
        p = variant_value.get<std::string>();
    } else if (variant_type == "unsigned int") {
        p = variant_value.get<uint>();
    } else if (variant_type == "char") {
        p = variant_value.get<char>();
    } else if (variant_type == "unsigned char") {
        p = variant_value.get<uchar>();
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }
}

RuleSection& RuleSection::EnableStringSanitization()
{
    SanitizeIniStrings = true;
    return *this;
}

RuleSection& RuleSection::DisableStringSanitization()
{
    SanitizeIniStrings = false;
    return *this;
}

bool RuleSection::Variants_Have_Same_Type(const RuleValueVariant& value_variant_a, const RuleValueVariant& value_variant_b)
{
    if (std::holds_alternative<int>(value_variant_a)) {
        return std::holds_alternative<int>(value_variant_b);
    }
    if (std::holds_alternative<bool>(value_variant_a)) {
        return std::holds_alternative<bool>(value_variant_b);
    }
    if (std::holds_alternative<float>(value_variant_a)) {
        return std::holds_alternative<float>(value_variant_b);
    }
    if (std::holds_alternative<ushort>(value_variant_a)) {
        return std::holds_alternative<ushort>(value_variant_b);
    }
    if (std::holds_alternative<std::string>(value_variant_a)) {
        return std::holds_alternative<std::string>(value_variant_b);
    }
    if (std::holds_alternative<uint>(value_variant_a)) {
        return std::holds_alternative<uint>(value_variant_b);
    }
    if (std::holds_alternative<char>(value_variant_a)) {
        return std::holds_alternative<char>(value_variant_b);
    }
    if (std::holds_alternative<uchar>(value_variant_a)) {
        return std::holds_alternative<uchar>(value_variant_b);
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string_view RuleSection::Get_Variant_Type(const RuleValueVariant& value_variant)
{
    if (std::holds_alternative<int>(value_variant)) {
        return "int";
    }
    if (std::holds_alternative<bool>(value_variant)) {
        return "bool";
    }
    if (std::holds_alternative<float>(value_variant)) {
        return "float";
    }
    if (std::holds_alternative<ushort>(value_variant)) {
        return "unsigned short";
    }
    if (std::holds_alternative<std::string>(value_variant)) {
        return "string";
    }
    if (std::holds_alternative<uint>(value_variant)) {
        return "unsigned int";
    }
    if (std::holds_alternative<char>(value_variant)) {
        return "char";
    }
    if (std::holds_alternative<uchar>(value_variant)) {
        return "unsigned char";
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string RuleSection::Get_Variant_Values(const RuleValueVariant& value_variant)
{
    if (std::holds_alternative<int>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    }
    if (std::holds_alternative<bool>(value_variant)) {
        return "true/false";
    }
    if (std::holds_alternative<float>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
    }
    if (std::holds_alternative<ushort>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<ushort>::min(), std::numeric_limits<ushort>::max());
    }
    if (std::holds_alternative<std::string>(value_variant)) {
        return "anything";
    }
    if (std::holds_alternative<uint>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<uint>::min(), std::numeric_limits<uint>::max());
    }
    if (std::holds_alternative<char>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
    }
    if (std::holds_alternative<uchar>(value_variant)) {
        return std::format("{}-{}", std::numeric_limits<uchar>::min(), std::numeric_limits<uchar>::max());
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string RuleSection::Variant_To_String(const RuleValueVariant& value_variant)
{
    if (const auto value = std::get_if<int>(&value_variant)) {
        return std::format("{}", *value);
    }
    if (const auto value = std::get_if<bool>(&value_variant)) {
        return std::format("{}", *value);
    }
    if (const auto value = std::get_if<float>(&value_variant)) {
        return std::format("{}", *value);
    }
    if (const auto value = std::get_if<ushort>(&value_variant)) {
        return std::format("{}", *value);
    }
    if (const auto value = std::get_if<std::string>(&value_variant)) {
        return *value;
    }
    if (const auto value = std::get_if<uint>(&value_variant)) {
        return std::format("{}", *value);
    }
    if (const auto value = std::get_if<char>(&value_variant)) {
        return std::format("{}", static_cast<int>(*value));
    }
    if (const auto value = std::get_if<uchar>(&value_variant)) {
        return std::format("{}", static_cast<unsigned int>(*value));
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

bool RuleSection::Has_Key(std::string_view name) const
{
    return Rules.contains(name.data());
}

std::vector<std::string_view> RuleSection::Rule_Names() const
{
    std::vector<std::string_view> keys;
    keys.reserve(Rules.size());

    for (const auto& key : Rules | std::views::keys) {
        keys.emplace_back(key);
    }

    return keys;
}

RuleValueVariant RuleSection::Get_Variant(std::string_view name) const
{
    const auto& it = Rules.find(name.data());

    if (it == Rules.end()) {
        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    return it->second;
}

std::optional<RuleValueVariant> RuleSection::Try_Get_Variant(std::string_view name) const
{
    const auto& it = Rules.find(name.data());

    if (it != Rules.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::string_view RuleSection::Get_Type(std::string_view name) const
{
    const auto& it = Rules.find(name.data());

    if (it == Rules.end()) {
        CNC_LOGGER_FATAL("Rule not found in section: [{}] -> {}", SectionName, name);
    }

    return Get_Variant_Type(it->second);
}

RuleSection& RuleSection::Set_Ini_Comment(const std::string& comment)
{
    if (CncStringUtils::Is_Blank(comment)) {
        CNC_LOGGER_DEBUG("Skipping blank INI comment for section: {}", SectionName);
        return *this;
    }

    Comment = comment;

    return *this;
}

const std::optional<std::string>& RuleSection::Get_Ini_Comment() const
{
    return Comment;
}

const RuleSection& RuleSection::Save_To_Ini(INIClass& ini, std::string_view name) const
{
    auto value_variant = Get_Variant(name);

    CNC_LOGGER_DEBUG(
        "Exporting rule to INI: [{}] -> {} = {}",
        SectionName,
        name,
        Variant_To_String(value_variant)
    );

    const auto comment = Try_Get_Rule_Comment(name);

    if (const auto value = std::get_if<int>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value, 0, comment);
    } else if (const auto value = std::get_if<bool>(&value_variant)) {
        ini.Put_Bool(SectionName.data(), name.data(), *value, comment);
    } else if (const auto value = std::get_if<float>(&value_variant)) {
        const auto value_str = std::format("{}", *value);
        ini.Put_String(SectionName.data(), name.data(), value_str, comment);
    } else if (const auto value = std::get_if<ushort>(&value_variant)) {
        const auto value_str = std::format("{}", static_cast<int>(*value));
        ini.Put_String(SectionName.data(), name.data(), value_str, comment);
    } else if (const auto value = std::get_if<uint>(&value_variant)) {
        const auto value_str = std::format("{}", *value);
        ini.Put_String(SectionName.data(), name.data(), value_str, comment);
    } else if (const auto value = std::get_if<char>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value, 0, comment);
    } else if (const auto value = std::get_if<uchar>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value, 0, comment);
    } else if (const auto value = std::get_if<std::string>(&value_variant)) {
        ini.Put_String(SectionName.data(), name.data(), *value, comment);
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }

    return *this;
}

void RuleSection::Save_All_To_Ini(INIClass& ini) const
{
    ini.Put_Section(SectionName.c_str(), Comment);

    for (const auto& key : Rules | std::views::keys) {
        Save_To_Ini(ini, key);
    }
}

size_t RuleSection::Get_C_Str(const std::string_view name, char* buffer, const size_t buffer_size) const
{
    const auto str_value = Get<std::string>(name);
    const auto str_length = str_value.length();

    buffer[0] = '\0';
    strncpy(buffer, str_value.data(), buffer_size);

    if (str_length >= buffer_size) {
        buffer[buffer_size - 1] = '\0';
    }

    return str_length;
}

RuleSection& RuleSection::Set(std::string_view name, RuleValueVariant value)
{
    CNC_LOGGER_DEBUG(
        "Updating rule at runtime: [{}] -> {} = {}",
        SectionName,
        name,
        Variant_To_String(value)
    );

    const auto existing_rule = Try_Get_Variant(name);

    if (existing_rule.has_value()) {
        if (!Variants_Have_Same_Type(existing_rule.value(), value)) {
            CNC_LOGGER_FATAL(
                "Attempted to set rule using wrong type '{}' (correct type: {}), found in section: [{}] -> {}",
                Get_Variant_Type(value),
                Get_Variant_Type(existing_rule.value()),
                SectionName,
                name
            );
        }
    }

    Rules[name.data()] = value;

    OnRulesChanged(*this, name, value);

    return *this;
}

RuleSection& RuleSection::Set(std::string_view name, const char* value)
{
    return Set(name, std::string(value));
}

const std::optional<std::string>& RuleSection::Get_Converter_Section_Type_Name()
{
    return ConverterSectionTypeName;
}

RuleSection& RuleSection::Set_Rule_Comment(const std::string_view name, std::string comment)
{
    RuleComments[name.data()] = std::move(comment);

    return *this;
}

std::optional<std::string> RuleSection::Try_Get_Rule_Comment(const std::string_view name) const
{
    if (RuleComments.contains(name.data())) {
        return RuleComments.at(name.data());
    }

    return std::nullopt;
}

TO_JSON(RuleSection)
{
    for (const auto& [ name, value ] : p.Rules) {
        to_json(j[name], value);
    }
}

/**
 * Update rules from JSON export, values for existing rules not present in the JSON export are preserved.
 */
FROM_JSON(RuleSection)
{
    if (!j.is_object()) {
        throw CncJsonException(
            "Invalid {} JSON - expected type 'object', actual type: {}",
            NAMEOF(RuleSection),
            j.type_name()
        );
    }

    for (const auto& [ key, value ] : j.items()) {
        RuleValueVariant variant_value;

        from_json(value, variant_value);

        p.Set(key, std::move(variant_value));
    }
}

//IniRuleContext
const IniRuleContext& IniRuleContext::Save(std::string_view name) const
{
    Section.Save_To_Ini(Context, name);

    return *this;
}

IniRuleContext& IniRuleContext::Load(std::string_view name)
{
    NameInStream = std::make_optional(name);

    return *this;
}

// RuleSections

void RuleSections::Default_Rules_Changed_Handler(std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed)
{
    OnRulesChangedDefault = std::move(on_rules_changed);
}

std::vector<std::string_view> RuleSections::Section_Names() const
{
    std::vector<std::string_view> keys;
    keys.reserve(Sections.size());

    for (const auto& key : Sections | std::views::keys) {
        keys.emplace_back(key);
    }

    return keys;
}

bool RuleSections::Has_Section(std::string_view name) const
{
    return Sections.contains(name.data());
}

void RuleSections::Save_All_To_Ini(INIClass& ini) const
{
    for (const auto& section : Sections | std::views::values) {
        section.Save_All_To_Ini(ini);
    }
}

RuleSection& RuleSections::Add_Section(std::string_view name, std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed)
{
    CNC_LOGGER_DEBUG("Adding new rules section '{}'", name);

    Sections.emplace(name.data(), RuleSection(
        name.data(),
        std::move(on_rules_changed)
    ));

    return Sections.at(name.data());
}

RuleSection& RuleSections::Add_Section(std::string_view name)
{
    return Add_Section(name, [&](auto& s, auto r, const auto& v) {
        if (OnRulesChangedDefault.has_value()) {
            OnRulesChangedDefault.value()(s, r, v);
        }
    });
}

RuleSection& RuleSections::operator[](std::string_view name)
{
    if (!Sections.contains(name.data())) {
        return Add_Section(name.data());
    }

    return Sections.at(name.data());
}

const RuleSection& RuleSections::operator[](std::string_view name) const
{
    if (!Sections.contains(name.data())) {
        throw std::out_of_range(std::format("Attempted to access missing rule section: {}'", name));
    }

    return Sections.at(name.data());
}

TO_JSON(RuleSections)
{
    for (const auto& [ name, section ] : p.Sections) {
        j[name] = section;
    }
}

/**
 * Read rule sections from JSON, RuleSections::[] only calls Add_Section if the section is not present, so existing
 * sections will be updated rather than replaced.
 */
FROM_JSON(RuleSections)
{
    if (!j.is_object()) {
        throw CncJsonException(
            "Invalid {} JSON - expected type 'object', actual type: {}",
            NAMEOF(RuleSections),
            j.type_name()
        );
    }

    for (const auto& [name, section_json ] : j.items()) {
        from_json(section_json, p[name]);
    }
}
