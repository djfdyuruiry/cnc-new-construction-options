#pragma once

#include <format>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <lua.hpp>

#include "../logger.h"
#include "luaengine.h"
#include "luaresult.h"

/**
 * Wrapper around a map of LuaVariant to help
 * validate table fields before unpacking them
 * to concrete types.
 */
class LuaMapParameter
{
public:
    LuaMapParameter(
        const LuaEngine& lua,
        std::string_view function_signature,
        std::string_view parameter,
        std::map<std::string,
        LuaVariant> data
    ): Lua(lua), FunctionSignature(function_signature), Parameter(parameter), Data(data) {}

    template<class T>
    LuaMapParameter& With_Key(std::string key)
    {
        if (Data.find(key) == Data.end()) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' didn't contain expected key: {}",
                FunctionSignature,
                Parameter,
                key
            );
        }

        auto value_ptr = std::get_if<T>(&Data[key]);
        if (!value_ptr) {
            auto expected_lua_type = LUA_TNONE;

            if constexpr (std::is_same_v<T, std::string>) {
                expected_lua_type = LUA_TSTRING;
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                expected_lua_type = LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                expected_lua_type = LUA_TBOOLEAN;
            } else {
                static_assert("Attempted to use non LuaVariant type when calling Assert_Key_Is");
            }

            Lua.Raise_Error_Format(
                "({}) Invalid type for key '{}' in parameter '{}', expected '{}' but got '{}'",
                FunctionSignature,
                key,
                Parameter,
                LuaEngine::LuaTypeMap[expected_lua_type].value(),
                Lua.Get_Variant_Type(Data[key])
            );
        }

        return *this;
    }

    template<class T>
    T Get(std::string key)
    {
        return std::get<T>(Data[key]);
    }

private:
    const LuaEngine& Lua;
    std::string_view FunctionSignature;
    std::string_view Parameter;
    std::map<std::string, LuaVariant> Data;
};

/**
 * Wrapper around a vector of LuaVariant to help
 * validate array items before unpacking them
 * to concrete types.
 */
class LuaArrayParameter
{
public:
    LuaArrayParameter(
        const LuaEngine& lua,
        std::string_view function_signature,
        std::string_view parameter,
        std::vector<LuaVariant> data
    ): Lua(lua), FunctionSignature(function_signature), Parameter(parameter), Data(data) {}

    std::size_t Get_Size()
    {
        return Data.size();
    }

    template<class T>
    const LuaArrayParameter& With_Index(int idx)
    {
        if (Data.size() < idx + 1) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' size is too small for expected index: {}",
                FunctionSignature,
                Parameter,
                idx
            );
        }

        auto value_ptr = std::get_if<T>(&Data[idx]);

        if (!value_ptr) {
            auto expected_lua_type = LUA_TNONE;

            if constexpr (std::is_same_v<T, std::string>) {
                expected_lua_type = LUA_TSTRING;
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                expected_lua_type = LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                expected_lua_type = LUA_TBOOLEAN;
            } else {
                static_assert("Attempted to use non LuaVariant type when calling Assert_Index_Is");
            }

            Lua.Raise_Error_Format(
                "({}) Invalid type for index '{}' in parameter '{}', expected '{}' but got '{}'",
                FunctionSignature,
                idx,
                Parameter,
                LuaEngine::LuaTypeMap[expected_lua_type].value(),
                Lua.Get_Variant_Type(Data[idx])
            );
        }

        return *this;
    }

    // TODO: With_Values<T> to assert entire array is of type

    template<class T>
    T Get(int idx)
    {
        return std::get<T>(Data[idx]);
    }

private:
    const LuaEngine& Lua;
    std::string_view FunctionSignature;
    std::string_view Parameter;
    std::vector<LuaVariant> Data;
};

/**
 * Wrapper around LuaEngine to validate and read arguments passed to a CFunction.
 * 
 * Fluent interface available for validating arguments:
 *   Call Count_Is and one or more X_Argument_Is(Not_Y) then finish with Assert
 * 
 * Fluent interface available for reading arguments:
 *   Call Read_First then zero or more Read_Next. Arguments can be peeked with
 *   First_Read_Is and Next_Read_Is to check upcoming types (useful for multi type args)
 */
class LuaArguments
{
public:
    const int Count;
    const std::string FunctionSignature;

    LuaArguments(const LuaEngine& lua, std::string function_signature) : 
        Lua(lua),
        Count(lua.Get_Stack_Count()),
        FunctionSignature(function_signature) {}

    // Fluent assert stream methods

    LuaArguments& Count_Is(int expected)
    {
        if (StreamIsValid.has_value() && !StreamIsValid.value()) {
            // already invalid, short circuit
            return *this;
        }

        StreamIsValid = Count == expected;
        StreamArgumentIndex = 1;

        return *this;
    }

    template<class T>
    LuaArguments& Next_Argument_Is()
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

        StreamIsValid = Lua.template Is_Type<T>(StreamArgumentIndex.value());
        StreamArgumentIndex = StreamArgumentIndex.value() + 1;

        return *this;
    }

    template<class T>
    LuaArguments& First_Argument_Is()
    {
        StreamArgumentIndex = 1;

        return Next_Argument_Is<T>();
    }

    LuaArguments& Next_Argument_Is_Not_Nil()
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

    LuaArguments& First_Argument_Is_Not_Nil()
    {
        StreamArgumentIndex = 1;

        return Next_Argument_Is_Not_Nil();
    }

    LuaArguments& Next_Argument_Is_Not_None()
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

    LuaArguments& First_Argument_Is_Not_None()
    {
        StreamArgumentIndex = 1;

        return Next_Argument_Is_Not_None();
    }

    LuaArguments& Next_Argument_Is_Table()
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

    LuaArguments& First_Argument_Is_Table()
    {
        StreamArgumentIndex = 1;

        return Next_Argument_Is_Table();
    }

    bool Assert()
    {
        if (!StreamIsValid.has_value()) {
            // called before Is calls, assume invalid
            StreamIsValid = false;
        }

        auto result = StreamIsValid.value();

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

    LuaArrayParameter Read_Next_Array(std::string_view parameter_name)
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

            Lua.Type_Read_Variant(-1)
                .If_Value([&](auto value) {
                    table_array.emplace_back(value);
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

        return LuaArrayParameter(Lua, FunctionSignature, parameter_name, table_array);
    }

    LuaArrayParameter Read_First_Array(std::string_view parameter_name) {
        ReadStreamArgumentIndex = 1;

        return Read_Next_Array(parameter_name);
    }

    LuaMapParameter Read_Next_Map(std::string_view parameter_name)
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        if (ReadStreamArgumentIndex > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
        }

        std::map<std::string, LuaVariant> table_map;
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
                    Lua.Type_Read_Variant(-1)
                        .If_Value([&](auto value) {
                            table_map[key] = value;
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

        return LuaMapParameter(Lua, FunctionSignature, parameter_name, table_map);
    }

    LuaMapParameter Read_First_Map(std::string_view parameter_name)
    {
        ReadStreamArgumentIndex = 1;

        return Read_Next_Map(parameter_name);
    }

    const std::string_view Get_Next_Read_Type()
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        return Lua.Get_Lua_Type(ReadStreamArgumentIndex.value());
    }

    const std::string_view First_Read_Type()
    {
        ReadStreamArgumentIndex = 1;

        return Lua.Get_Lua_Type(ReadStreamArgumentIndex.value());
    }

    template<class T>
    bool Next_Read_Is()
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        auto result = Lua.template Is_Type<T>(ReadStreamArgumentIndex.value());

        return result;
    }

    template<class T>
    LuaResultWithValue<T> First_Read_Is()
    {
        ReadStreamArgumentIndex = 1;

        return Next_Is<T>();
    }

    LuaResultWithValue<LuaVariant> Read_Next_Variant()
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        if (ReadStreamArgumentIndex > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
        }

        auto result = Lua.Type_Read_Variant(ReadStreamArgumentIndex.value());

        ReadStreamArgumentIndex = ReadStreamArgumentIndex.value() + 1;

        return result;
    }

    LuaResultWithValue<LuaVariant> Read_First_Variant()
    {
        ReadStreamArgumentIndex = 1;

        return Read_Next_Variant();
    }

    template<class T>
    LuaResultWithValue<T> Read_Next()
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        if (ReadStreamArgumentIndex > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", FunctionSignature);
        }

        auto result = Lua.Try_Read<T>(ReadStreamArgumentIndex.value());

        ReadStreamArgumentIndex = ReadStreamArgumentIndex.value() + 1;

        return result;
    }

    template<class T>
    LuaResultWithValue<T> Read_First()
    {
        ReadStreamArgumentIndex = 1;

        return Read_Next<T>();
    }


    void Assert_String_Parameter_Is_Valid(std::string_view name, std::string value)
    {
        return Assert_String_Parameter_Is_Valid(name, value, INT_MAX);
    }

    void Assert_String_Parameter_Is_Valid(std::string_view name, std::string value, unsigned int max_chars)
    {
        if (value.empty() || std::all_of(value.begin(), value.end(), ::isspace)) {
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
private:
    const LuaEngine& Lua;
    std::optional<bool> StreamIsValid;
    std::optional<int> StreamArgumentIndex;

    std::optional<int> ReadStreamArgumentIndex;
};
