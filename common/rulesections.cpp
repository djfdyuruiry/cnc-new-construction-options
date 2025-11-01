#include <ranges>

#include "rulesections.h"

bool RuleSection::Variants_Have_Same_Type(RuleValueVariant value_variant_a, RuleValueVariant value_variant_b)
{
    if (std::get_if<int>(&value_variant_a)) {
        return std::get_if<int>(&value_variant_b);
    }
    if (std::get_if<bool>(&value_variant_a)) {
        return std::get_if<bool>(&value_variant_b);
    }
    if (std::get_if<float>(&value_variant_a)) {
        return std::get_if<float>(&value_variant_b);
    }
    if (std::get_if<ushort>(&value_variant_a)) {
        return std::get_if<ushort>(&value_variant_b);
    }
    if (std::get_if<std::string>(&value_variant_a)) {
        return std::get_if<std::string>(&value_variant_b);
    }
    if (std::get_if<uint>(&value_variant_a)) {
        return std::get_if<uint>(&value_variant_b);
    }
    if (std::get_if<char>(&value_variant_a)) {
        return std::get_if<char>(&value_variant_b);
    }
    if (std::get_if<uchar>(&value_variant_a)) {
        return std::get_if<uchar>(&value_variant_b);
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string_view RuleSection::Get_Variant_Type(RuleValueVariant value_variant)
{
    if (std::get_if<int>(&value_variant)) {
        return "int";
    }
    if (std::get_if<bool>(&value_variant)) {
        return "bool";
    }
    if (std::get_if<float>(&value_variant)) {
        return "float";
    }
    if (std::get_if<ushort>(&value_variant)) {
        return "unsigned short";
    }
    if (std::get_if<std::string>(&value_variant)) {
        return "string";
    }
    if (std::get_if<uint>(&value_variant)) {
        return "unsigned int";
    }
    if (std::get_if<char>(&value_variant)) {
        return "char";
    }
    if (std::get_if<uchar>(&value_variant)) {
        return "unsigned char";
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string RuleSection::Get_Variant_Values(RuleValueVariant value_variant)
{
    if (std::get_if<int>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    }
    if (std::get_if<bool>(&value_variant)) {
        return "true/false";
    }
    if (std::get_if<float>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
    }
    if (std::get_if<ushort>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<ushort>::min(), std::numeric_limits<ushort>::max());
    }
    if (std::get_if<std::string>(&value_variant)) {
        return "anything";
    }
    if (std::get_if<uint>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<uint>::min(), std::numeric_limits<uint>::max());
    }
    if (std::get_if<char>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
    }
    if (std::get_if<uchar>(&value_variant)) {
        return std::format("{}-{}", std::numeric_limits<uchar>::min(), std::numeric_limits<uchar>::max());
    }

    throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
}

std::string RuleSection::Variant_To_String(RuleValueVariant value_variant)
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

RuleSection& RuleSection::Set_Ini_Comment(INIClass& ini, const std::string& comment)
{
    if (CncStringUtils::Is_Blank(comment)) {
        CNC_LOGGER_DEBUG("Skipping blank INI comment for section: {}", SectionName);
        return *this;
    }

    ini.Put_Comment(SectionName.c_str(), comment);

    return *this;
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

    if (const auto value = std::get_if<int>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value);
    } else if (const auto value = std::get_if<bool>(&value_variant)) {
        ini.Put_Bool(SectionName.data(), name.data(), *value);
    } else if (const auto value = std::get_if<float>(&value_variant)) {
        const auto value_str = std::format("{}", *value);
        ini.Put_String(SectionName.data(), name.data(), value_str);
    } else if (const auto value = std::get_if<ushort>(&value_variant)) {
        const auto value_str = std::format("{}", static_cast<int>(*value));
        ini.Put_String(SectionName.data(), name.data(), value_str);
    } else if (const auto value = std::get_if<uint>(&value_variant)) {
        const auto value_str = std::format("{}", *value);
        ini.Put_String(SectionName.data(), name.data(), value_str);
    } else if (const auto value = std::get_if<char>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value);
    } else if (const auto value = std::get_if<uchar>(&value_variant)) {
        ini.Put_Int(SectionName.data(), name.data(), *value);
    } else if (const auto value = std::get_if<std::string>(&value_variant)) {
        ini.Put_String(SectionName.data(), name.data(), *value);
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }

    return *this;
}

void RuleSection::Save_All_To_Ini(INIClass& ini) const
{
    for (const auto& key : Rules | std::views::keys) {
        Save_To_Ini(ini, key);
    }
}

RuleSection& RuleSection::Set(std::string_view name, RuleValueVariant value)
{
    CNC_LOGGER_WARN(
        "Updating rule at runtime: [{}] -> {} = {}",
        SectionName,
        name,
        Variant_To_String(value)
    );

    auto existing_rule = Try_Get_Variant(name);

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

    CNC_LOGGER_WARN("Running OnRulesChanged() handler");
    OnRulesChanged(*this, name, value);

    return *this;
}

std::optional<std::string_view>& RuleSection::Get_Converter_Section_Type_Name()
{
    return ConverterSectionTypeName;
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
    OnRulesChangedDefault = on_rules_changed;
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
        section->Save_All_To_Ini(ini);
    }
}

RuleSection& RuleSections::Add_Section(std::string_view name, std::function<void(RuleSection&, std::string_view, const RuleValueVariant&)> on_rules_changed)
{
    CNC_LOGGER_DEBUG("Adding new rules section '{}'", name);

    Sections[name.data()] = std::make_unique<RuleSection>(
        name.data(),
        on_rules_changed
    );

    return *Sections[name.data()];
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
    auto it = Sections.find(name.data());

    if (it != Sections.end()) {
        return *(it->second);
    }

    return Add_Section(name.data());
}
