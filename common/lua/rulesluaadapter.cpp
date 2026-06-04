#include "rulesluaadapter.h"

void RulesLuaAdapter::Push_Rule_Type(
    const LuaEngine& engine,
    const RuleSections& sections,
    std::string section,
    std::string key
)
{
    Assert_Rule_Exists(engine, sections, section, key);

    const auto& rule_value_variant = sections[section].Get_Variant(key);

    if (
        std::holds_alternative<int>(rule_value_variant) ||
        std::holds_alternative<uint>(rule_value_variant) ||
        std::holds_alternative<float>(rule_value_variant) ||
        std::holds_alternative<ushort>(rule_value_variant) ||
        std::holds_alternative<char>(rule_value_variant) ||
        std::holds_alternative<uchar>(rule_value_variant)
    ) {
        engine.Push_Value(
            LuaEngine::LuaTypeMap[LUA_TNUMBER].value()
        );
    } else if (std::holds_alternative<bool>(rule_value_variant)) {
        engine.Push_Value(
            LuaEngine::LuaTypeMap[LUA_TBOOLEAN].value()
        );
    } else if (std::holds_alternative<std::string>(rule_value_variant)) {
        engine.Push_Value(
            LuaEngine::LuaTypeMap[LUA_TSTRING].value()
        );
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }
}

void RulesLuaAdapter::Push_Rule_Variant(const LuaEngine& engine, const RuleValueVariant& variant)
{
    if (const auto value = std::get_if<int>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<uint>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<char>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<uchar>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<ushort>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<float>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<bool>(&variant)) {
        engine.Push_Value(*value);
    } else if (const auto value = std::get_if<std::string>(&variant)) {
        engine.Push_Value(*value);
    } else {
        throw std::invalid_argument("Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without updating supporting code");
    }
}

void RulesLuaAdapter::Push_Rule_Value(
    const LuaEngine& engine,
    const RuleSections& sections,
    std::string section,
    std::string key
)
{
    Assert_Rule_Exists(engine, sections, section, key);
    Push_Rule_Variant(engine, sections[section].Get_Variant(key));
}

void RulesLuaAdapter::Set_Rule_Value(
    const SharedLuaEngine& engine,
    LuaArguments& arguments,
    const RuleSections& sections,
    const std::string& section,
    const std::string& key
)
{
    Assert_Rule_Exists(engine, sections, section, key);

    // unpack variant to call corresponding section Set template
    const auto& rule_value_variant = sections[section].Get_Variant(key);
    auto rule_type_error = true;
    auto expected_type = LUA_TNONE;

    if (
        std::holds_alternative<int>(rule_value_variant) ||
        std::holds_alternative<uint>(rule_value_variant) ||
        std::holds_alternative<ushort>(rule_value_variant) ||
        std::holds_alternative<char>(rule_value_variant) ||
        std::holds_alternative<uchar>(rule_value_variant)
    ) {
        expected_type = LUA_TNUMBER;
    }

    if (std::holds_alternative<int>(rule_value_variant)) {
        rule_type_error = !Set_Rule_Value_For_Section<int, int>(arguments, sections[section], key);
    } else if (std::holds_alternative<uint>(rule_value_variant)) {
        rule_type_error = !Set_Rule_Value_For_Section<int, uint>(arguments, sections[section], key);
    } else if (std::holds_alternative<ushort>(rule_value_variant)) {
        rule_type_error = !Set_Rule_Value_For_Section<int, ushort>(arguments, sections[section], key);
    } else if (std::holds_alternative<char>(rule_value_variant)) {
        rule_type_error = !Set_Rule_Value_For_Section<int, char>(arguments, sections[section], key);
    } else if (std::holds_alternative<uchar>(rule_value_variant)) {
        rule_type_error = !Set_Rule_Value_For_Section<int, uchar>(arguments, sections[section], key);
    } else if (std::holds_alternative<float>(rule_value_variant)) {
        expected_type = LUA_TNUMBER;

        if (arguments.Next_Read_Is<float>()) {
            sections[section].Set(
                key,
                arguments.Read_Next<float>().Unpack()
            );
            rule_type_error = false;
        }
    } else if (std::holds_alternative<bool>(rule_value_variant)) {
        expected_type = LUA_TBOOLEAN;

        if (arguments.Next_Read_Is<bool>()) {
            sections[section].Set(
                key,
                arguments.Read_Next<bool>().Unpack()
            );
            rule_type_error = false;
        }
    } else if (std::holds_alternative<std::string>(rule_value_variant)) {
        expected_type = LUA_TSTRING;

        if (arguments.Next_Read_Is<std::string>()) {
            sections[section].Set(
                key,
                arguments.Read_Next<std::string>().Unpack()
            );
            rule_type_error = false;
        }
    } else {
        throw std::invalid_argument(
            "Unsupported RuleValueVariant type - this is normally caused by variant type list being updated without "
            "updating supporting code"
        );
    }

    if (rule_type_error) {
        engine.Raise_Error_Format(
            "Incorrect type passed for rule value, expected '{}' but got: {}",
            LuaEngine::LuaTypeMap[expected_type].value(),
            arguments.Get_Next_Read_Type()
        );
    }

    // return old rule value to lua caller
    Push_Rule_Variant(engine, rule_value_variant);
}

void RulesLuaAdapter::Assert_Rule_Exists(
    const LuaEngine& engine,
    const RuleSections& sections,
    std::string section,
    std::string key
)
{
    if (!sections.Has_Section(section)) {
        engine.Raise_Error_Format(
            "Rule section does not exist: {}",
            section
        );
    }

    if (!sections[section].Has_Key(key)) {
        engine.Raise_Error_Format(
            "Rule key does not exist in section '{}': {}",
            section,
            key
        );
    }
}
