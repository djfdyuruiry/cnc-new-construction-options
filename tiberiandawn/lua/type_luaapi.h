#pragma once

#include "externs.h"
#include "td_luaapi.h"
#include "type.h"
#include "typeconverter.h"
#include "lua/rulesluaadapter.h"

template<class T>
class TiberianDawnTypeLuaApi : protected TiberianDawnLuaApi
{
public:

    void Register_Functions(LuaEngine& engine) const override
    {
        With_Api_Namespace(engine, [](auto& n) {
            n.addCFunction("getProperty", [](auto L) {
                const auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "{}.getProperty(<string: instanceName>, <string: propertyName>)");

                arguments.Count_Is(2)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto instance_name = arguments.Read_First<std::string>().Unpack();
                auto property_name = arguments.Read_Next<std::string>().Unpack();

                auto instance = TdTypeConverter::Try_Parse<InfantryType>(instance_name);

                if (!instance.has_value()) {
                    engine.Raise_Error_Format("Unable to parse string as a type of {}: {}", "Infantry", instance_name);
                }

                auto class_instance = InfantryTypeClass::As_Reference(instance.value());

                RulesLuaAdapter::Push_Rule_Value(
                    engine,
                    Rule.Infantry,
                    class_instance.Name(),
                    property_name
                );

                return 1;
            })
            .addCFunction("setProperty", [](auto L) {
                const auto engine = SharedLuaEngine(L);
                auto arguments = LuaArguments(engine, "{}.getProperty(<string: instanceName>, <string: propertyName>, <any: propertyValue>)");

                arguments.Count_Is(3)
                    .First_Argument_Is<std::string>()
                    .Next_Argument_Is<std::string>()
                    .Assert();

                auto instance_name = arguments.Read_First<std::string>().Unpack();
                auto property_name = arguments.Read_Next<std::string>().Unpack();

                auto instance = TdTypeConverter::Try_Parse<InfantryType>(instance_name);

                if (!instance.has_value()) {
                    engine.Raise_Error_Format("Unable to parse string as a type of {}: {}", "Infantry", instance_name);
                }

                auto class_instance = InfantryTypeClass::As_Reference(instance.value());

                if (TdTypeConverter::Rule_Requires_Converter(class_instance.Read_INI(), property_name)) {

                } else {
                    RulesLuaAdapter::Set_Rule_Value(
                        engine,
                        arguments,
                        Rule.Infantry,
                        class_instance.Name(),
                        property_name
                    );
                }

                return 1;
            });
        });
    }
protected:
    const char* Get_Cpp_Source() const override
    {
        return __FILE__;
    }
};