#pragma once

#include "../../common/lua/rulesluaadapter.h"

#include "externs.h"
#include "rules.h"
#include "td_luaapi.h"
#include "typeconverter.h"

class TiberianDawnTypesLuaApi : public TiberianDawnLuaApi
{
public:
    TiberianDawnTypesLuaApi() : TiberianDawnLuaApi("Types") {}

    void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [&](auto& n) {
            Register_Type_Functions<AnimType, AnimTypeClass>(n);
            Register_Type_Functions<WarheadType, WarheadTypeClass>(n);
            Register_Type_Functions<BulletType, BulletTypeClass>(n);
            Register_Type_Functions<WeaponType, WeaponTypeClass>(n);
            Register_Type_Functions<AircraftType, AircraftTypeClass>(n);
            Register_Type_Functions<StructType, BuildingTypeClass>(n);
            Register_Type_Functions<InfantryType, InfantryTypeClass>(n);
            Register_Type_Functions<UnitType, UnitTypeClass>(n);
            Register_Type_Functions<HousesType, HouseTypeClass>(n);
        });
    }

protected:
    const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }

private:
    template<EnumSignedChar T, RulesTypeClass<T> U>
    void Register_Type_Functions(luabridge::Namespace& n) const
    {
        auto type_name = TdTypeConverter::Get_Type_Name<T>();
        auto get_property_value_function = std::format("get{}PropertyValue", type_name);
        auto set_property_value_function = std::format("set{}PropertyValue", type_name);
        auto get_properties_function = std::format("get{}PropertyNames", type_name);

        n.addCFunction(get_properties_function.c_str(), [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto type_name = TdTypeConverter::Get_Type_Name<T>();

            // fetch rule names using first instance in rule cache
            auto& sections = Rule.Get_Rule_Sections_For_Type<T>();
            auto section_names = sections.Section_Names();

            if (section_names.empty()) {
                engine.Raise_Error_Format("Empty rules cache detected for type: {}", type_name);
            }

            auto rule_names = sections[section_names.front()].Rule_Names();

            // push property names table to caller
            auto properties_table = LuaTableBuilder(engine);

            for (const auto& name : rule_names) {
                properties_table.With_Index_Value(name);
            }

            return 1;
        })
        .addCFunction(get_property_value_function.c_str(), [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto type_name = TdTypeConverter::Get_Type_Name<T>();
            auto arguments = LuaArguments(
                engine,
                std::format("get{}PropertyValue(<string: instanceName>, <string: propertyName>)", type_name)
            );

            // process args
            arguments.Count_Is(2)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Assert();

            const auto instance_name = arguments.Read_First<std::string>().Unpack();
            const auto property_name = arguments.Read_Next<std::string>().Unpack();

            // validate arg
            auto instance = TdTypeConverter::Try_Parse<T>(instance_name);

            if (!instance.has_value()) {
                engine.Raise_Error_Format("Unable to parse string as a type of {}: {}", "Infantry", instance_name);
            }

            // fetch type instance and type rules
            auto& sections = Rule.Get_Rule_Sections_For_Type<T>();
            const auto& class_instance = U::As_Reference(instance.value());

            // push C++ primitive rule type as lua type (no conversion required, all rule cache types are primitives)
            RulesLuaAdapter::Push_Rule_Value(
                engine,
                sections,
                class_instance.Name(),
                property_name
            );

            return 1;
        })
        .addCFunction(set_property_value_function.c_str(), [](auto L) {
            const auto engine = SharedLuaEngine(L);
            auto type_name = TdTypeConverter::Get_Type_Name<T>();
            auto arguments = LuaArguments(
                engine,
                std::format("{}(<string: instanceName>, <string: propertyName>, <any: propertyValue>)", type_name)
            );

            // process args
            arguments.Count_Is(3)
                .First_Argument_Is<std::string>()
                .Next_Argument_Is<std::string>()
                .Next_Argument_Is_Not_Nil()
                .Assert();

            const auto instance_name = arguments.Read_First<std::string>().Unpack();
            const auto property_name = arguments.Read_Next<std::string>().Unpack();

            // validate arg
            const auto instance = TdTypeConverter::Try_Parse<T>(instance_name);

            if (!instance.has_value()) {
                engine.Raise_Error_Format("Unable to parse string as a type of {}: {}", type_name, instance_name);
            }

            // fetch type rules and instance name
            auto& sections = Rule.Get_Rule_Sections_For_Type<T>();
            auto section_name = U::As_Reference(instance.value()).Name();

            if (TdTypeConverter::Rule_Requires_Converter(type_name, property_name)) {
                // rule is of special type that needs conversion from a string value
                RulesLuaAdapter::Assert_Rule_Exists(engine, sections, section_name, property_name);

                // get new string value for the rule and rules section for specific type instance
                auto property_value = arguments.Read_Next<std::string>().Unpack();
                auto section = sections[section_name];

                // get current rule value and type
                auto current_value = section.template Get<std::string>(property_name);
                auto converter_variant = TdTypeConverter::Get_Rule_Variant(type_name, property_name);

                try {
                    // convert string and set rule value (class_instance is updated by OnRulesChanged handler in section)
                    TdTypeConverter::Set_Rule_With_Variant(
                        section,
                        property_name,
                        property_value,
                        converter_variant
                    );
                } catch (const std::invalid_argument& ex) {
                    // catch conversion errors and throw back to lua caller
                    engine.Raise_Error(ex.what());
                }

                // return old value to caller
                engine.Push_Value(current_value);
            } else {
                // rule is of a standard type, read primitive lua type into C++ type and set rule
                RulesLuaAdapter::Set_Rule_Value(
                    engine,
                    arguments,
                    sections,
                    section_name,
                    property_name
                );
            }

            return 1;
        });
    }
};
