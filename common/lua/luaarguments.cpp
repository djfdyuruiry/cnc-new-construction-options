#include <format>
#include <functional>

#include "stringutils.h"

#include "luaarguments.h"

std::size_t LuaArrayParameter::Get_Size() const
{
    return Data.size();
}

// Method implementations for LuaArguments class

// Fluent assert stream methods
LuaArguments& LuaArguments::Count_Is(const int expected)
{
    if (StreamIsValid.has_value() && !StreamIsValid.value()) {
        // already invalid, short circuit
        return *this;
    }

    StreamIsValid = Count == expected;
    StreamArgumentIndex = 1;

    return *this;
}

LuaArguments& LuaArguments::Next_Argument_Is_Not_Nil()
{
    if (StreamIsValid.has_value() && !StreamIsValid.value()) {
        // already invalid, short circuit
        return *this;
    }

    if (!StreamArgumentIndex.has_value()) {
        StreamArgumentIndex = 1;
    }

    if (StreamArgumentIndex > Count) {
        Lua.Raise_Error_Format("({}) CFunction attempted to validate more arguments than were expected", FunctionSignature);
    }

    StreamIsValid = !Lua.Is_Nil(StreamArgumentIndex.value());
    StreamArgumentIndex = StreamArgumentIndex.value() + 1;

    return *this;
}

LuaArguments& LuaArguments::First_Argument_Is_Not_Nil()
{
    StreamArgumentIndex = 1;

    return Next_Argument_Is_Not_Nil();
}

LuaArguments& LuaArguments::Next_Argument_Is_Not_None()
{
    if (StreamIsValid.has_value() && !StreamIsValid.value()) {
        // already invalid, short circuit
        return *this;
    }

    if (!StreamArgumentIndex.has_value()) {
        StreamArgumentIndex = 1;
    }

    if (StreamArgumentIndex > Count) {
        Lua.Raise_Error_Format("({}) CFunction attempted to validate more arguments than were expected", FunctionSignature);
    }

    StreamIsValid = !Lua.Is_None(StreamArgumentIndex.value());
    StreamArgumentIndex = StreamArgumentIndex.value() + 1;

    return *this;
}

LuaArguments& LuaArguments::First_Argument_Is_Not_None()
{
    StreamArgumentIndex = 1;

    return Next_Argument_Is_Not_None();
}

LuaArguments& LuaArguments::Next_Argument_Is_Table()
{
    if (StreamIsValid.has_value() && !StreamIsValid.value()) {
        // already invalid, short circuit
        return *this;
    }

    if (!StreamArgumentIndex.has_value()) {
        StreamArgumentIndex = 1;
    }

    if (StreamArgumentIndex > Count) {
        Lua.Raise_Error("CFunction attempted to validate more arguments than were expected");
    }

    StreamIsValid = Lua.Is_Table(StreamArgumentIndex.value());
    StreamArgumentIndex = StreamArgumentIndex.value() + 1;

    return *this;
}

LuaArguments& LuaArguments::First_Argument_Is_Table()
{
    StreamArgumentIndex = 1;

    return Next_Argument_Is_Table();
}

bool LuaArguments::Assert()
{
    if (!StreamIsValid.has_value()) {
        // called before Is calls, assume invalid
        StreamIsValid = false;
    }

    const auto result = StreamIsValid.value();

    if (!result) {
        Lua.Raise_Error(
            std::format(
                "Incorrect number of arguments, or argument type mis-match. Usage: {}",
                FunctionSignature
            )
        );
    }

    StreamIsValid.reset();
    StreamArgumentIndex.reset();

    return result;
}

// Fluent read stream methods

LuaArrayParameter LuaArguments::Read_Next_Array(std::string_view parameter_name)
{
    if (!ReadStreamArgumentIndex.has_value()) {
        ReadStreamArgumentIndex = 1;
    }

    if (ReadStreamArgumentIndex > Count) {
        Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
    }

    std::vector<LuaVariant> table_array;
    std::optional<std::string> read_error;

    Lua.Push_Nil();

    while (!read_error.has_value() && Lua.Iterate_Over_Table(ReadStreamArgumentIndex.value())) {
        if (!Lua.Is_Type<int>(-2)) {
            read_error = std::format(
                "Argument '{}' was not an array (number indexed table)",
                parameter_name
            );
        
            continue;
        }

        Lua.Try_Read_Variant(-1)
            .If_Value([&](auto value) {
                table_array.emplace_back(std::move(value));
            })
            .On_Error([&](auto& r) {
                read_error = r.Error_Message();
            });
    }

    Lua.Pop();

    if (read_error.has_value()) {
        Lua.Raise_Error_Format(
            "({}) Error reading {} parameter as array: {}",
            FunctionSignature,
            parameter_name,
            read_error.value()
        );
    }

    ReadStreamArgumentIndex = ReadStreamArgumentIndex.value() + 1;

    return {Lua, FunctionSignature, parameter_name, table_array};
}

LuaArrayParameter LuaArguments::Read_First_Array(std::string_view parameter_name)
{
    ReadStreamArgumentIndex = 1;

    return Read_Next_Array(parameter_name);
}

LuaMapParameter LuaArguments::Read_Next_Map(std::string_view parameter_name)
{
    if (!ReadStreamArgumentIndex.has_value()) {
        ReadStreamArgumentIndex = 1;
    }

    if (ReadStreamArgumentIndex > Count) {
        Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
    }

    std::unordered_map<std::string, LuaVariant> table_map;
    std::optional<std::string> read_error;

    Lua.Push_Nil();

    if (!Lua.Is_Table(ReadStreamArgumentIndex.value())) {
        read_error = std::format(
            "Argument '{}' was not a table",
            parameter_name
        );
    }

    while (!read_error.has_value() && Lua.Iterate_Over_Table(ReadStreamArgumentIndex.value())) {
        if (!Lua.Is_Type<std::string>(-2)) {
            read_error = std::format(
                "Argument '{}' was not a map (string indexed table)",
                parameter_name
            );

            continue;
        }

        Lua.Try_Read<std::string>(-2)
            .If_Value([&](auto key){ 
                Lua.Try_Read_Variant(-1)
                    .If_Value([&](auto value) {
                        table_map[key] = std::move(value);
                    })
                    .On_Error([&](auto& r) {
                        read_error = r.Error_Message();
                    });
            })
            .On_Error([&](auto& r) {
                read_error = r.Error_Message();
            });

        Lua.Pop();
    }

    if (read_error.has_value()) {
        Lua.Raise_Error_Format(
            "({}) Error reading {} parameter as map: {}",
            FunctionSignature,
            parameter_name,
            read_error.value()
        );
    }

    ReadStreamArgumentIndex = ReadStreamArgumentIndex.value() + 1;

    return {Lua, FunctionSignature, parameter_name, table_map};
}

LuaMapParameter LuaArguments::Read_First_Map(const std::string_view parameter_name)
{
    ReadStreamArgumentIndex = 1;

    return Read_Next_Map(parameter_name);
}

std::string_view LuaArguments::Get_Next_Read_Type()
{
    if (!ReadStreamArgumentIndex.has_value()) {
        ReadStreamArgumentIndex = 1;
    }

    return Lua.Get_Lua_Type(ReadStreamArgumentIndex.value());
}

std::string_view LuaArguments::First_Read_Type()
{
    ReadStreamArgumentIndex = 1;

    return Lua.Get_Lua_Type(ReadStreamArgumentIndex.value());
}

LuaResultWithValue<LuaVariant> LuaArguments::Read_Next_Variant()
{
    if (!ReadStreamArgumentIndex.has_value()) {
        ReadStreamArgumentIndex = 1;
    }

    if (ReadStreamArgumentIndex > Count) {
        Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
    }

    auto result = Lua.Try_Read_Variant(ReadStreamArgumentIndex.value());

    ReadStreamArgumentIndex = ReadStreamArgumentIndex.value() + 1;

    return result;
}

LuaResultWithValue<LuaVariant> LuaArguments::Read_First_Variant()
{
    ReadStreamArgumentIndex = 1;

    return Read_Next_Variant();
}

void LuaArguments::Assert_String_Parameter_Is_Valid(const std::string_view name, const std::string& value) const
{
    return Assert_String_Parameter_Is_Valid(name, value, INT_MAX);
}

void LuaArguments::Assert_String_Parameter_Is_Valid(std::string_view name, const std::string& value, unsigned int max_chars) const
{
    if (CncStringUtils::Is_Blank(value)) {
        Lua.Raise_Error_Format(
            "({}) Parameter '{}' was blank",
            FunctionSignature,
            name,
            max_chars
        );
    }

    if (value.length() > max_chars) {
        Lua.Raise_Error_Format(
            "({}) Parameter '{}' was too long, should be at most {} characters long. Value: {}",
            FunctionSignature,
            name,
            max_chars,
            value
        );
    }
}
