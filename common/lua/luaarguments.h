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
class LuaMapParameter {
public:
    LuaMapParameter(
        const LuaEngine& lua,
        std::string_view function_signature,
        std::string_view parameter,
        std::map<std::string,
        LuaVariant> data
    ): Lua(lua), Function_Signature(function_signature), Parameter(parameter), Data(data) {}

    template<class T>
    LuaMapParameter& With_Key(std::string key) {
        if (Data.find(key) == Data.end()) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' didn't contain expected key: {}",
                Function_Signature,
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
                Function_Signature,
                key,
                Parameter,
                LuaEngine::Lua_Type_Map[expected_lua_type].value(),
                Lua.Get_Variant_Type(Data[key])
            );
        }

        return *this;
    }

    template<class T>
    T Get(std::string key) {
        return std::get<T>(Data[key]);
    }

private:
    const LuaEngine& Lua;
    std::string_view Function_Signature;
    std::string_view Parameter;
    std::map<std::string, LuaVariant> Data;
};

/**
 * Wrapper around a vector of LuaVariant to help
 * validate array items before unpacking them
 * to concrete types.
 */
class LuaArrayParameter {
public:
    LuaArrayParameter(
        const LuaEngine& lua,
        std::string_view function_signature,
        std::string_view parameter,
        std::vector<LuaVariant> data
    ): Lua(lua), Function_Signature(function_signature), Parameter(parameter), Data(data) {}

    std::size_t Get_Size() {
        return Data.size();
    }

    template<class T>
    const LuaArrayParameter& With_Index(int idx) {
        if (Data.size() < idx + 1) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' size is too small for expected index: {}",
                Function_Signature,
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
                Function_Signature,
                idx,
                Parameter,
                LuaEngine::Lua_Type_Map[expected_lua_type].value(),
                Lua.Get_Variant_Type(Data[idx])
            );
        }

        return *this;
    }

    // TODO: With_Values<T> to assert entire array is of type

    template<class T>
    T Get(int idx) {
        return std::get<T>(Data[idx]);
    }

private:
    const LuaEngine& Lua;
    std::string_view Function_Signature;
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
class LuaArguments {
public:
    const int Count;
    const std::string Function_Signature;

    LuaArguments(const LuaEngine& lua, std::string function_signature) : 
        Lua(lua),
        Count(lua.Get_Stack_Count()),
        Function_Signature(function_signature) {}

    // Fluent assert stream methods

    LuaArguments& Count_Is(int expected) {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        Stream_Is_Valid = Count == expected;
        Stream_Argument_Index = 1;

        return *this;
    }

    template<class T>
    LuaArguments& Next_Argument_Is() {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        if (!Stream_Argument_Index.has_value()) {
            Stream_Argument_Index = 1;
        }

        if (Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to validate more arguments than were expected", Function_Signature);
        }

        Stream_Is_Valid = Lua.template Is_Type<T>(Stream_Argument_Index.value());
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    template<class T>
    LuaArguments& First_Argument_Is() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is<T>();
    }

    LuaArguments& Next_Argument_Is_Not_Nil() {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        if (!Stream_Argument_Index.has_value()) {
            Stream_Argument_Index = 1;
        }

        if (Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to validate more arguments than were expected", Function_Signature);
        }

        Stream_Is_Valid = !Lua.Is_Nil(Stream_Argument_Index.value());
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    LuaArguments& First_Argument_Is_Not_Nil() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is_Not_Nil();
    }

    LuaArguments& Next_Argument_Is_Not_None() {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        if (!Stream_Argument_Index.has_value()) {
            Stream_Argument_Index = 1;
        }

        if (Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to validate more arguments than were expected", Function_Signature);
        }

        Stream_Is_Valid = !Lua.Is_None(Stream_Argument_Index.value());
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    LuaArguments& First_Argument_Is_Not_None() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is_Not_None();
    }

    LuaArguments& Next_Argument_Is_Table() {
        if (Stream_Is_Valid.has_value() && !Stream_Is_Valid.value()) {
            // already invalid, short circuit
            return *this;
        }

        if (!Stream_Argument_Index.has_value()) {
            Stream_Argument_Index = 1;
        }

        if (Stream_Argument_Index > Count) {
            Lua.Raise_Error("CFunction attempted to validate more arguments than were expected");
        }

        Stream_Is_Valid = Lua.Is_Table(Stream_Argument_Index.value());
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    LuaArguments& First_Argument_Is_Table() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is_Table();
    }

    bool Assert() {
        if (!Stream_Is_Valid.has_value()) {
            // called before Is calls, assume invalid
            Stream_Is_Valid = false;
        }

        auto result = Stream_Is_Valid.value();

        if (!result) {
            Lua.Raise_Error(
                std::format(
                    "Incorrect number of arguments, or argument type mis-match. Usage: {}",
                    Function_Signature
                )
            );
        }

        Stream_Is_Valid.reset();
        Stream_Argument_Index.reset();

        return result;
    }

    // Fluent read stream methods

    LuaArrayParameter Read_Next_Array(std::string_view parameter_name) {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        if (Read_Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", Function_Signature);
        }

        std::vector<LuaVariant> table_array;
        std::optional<std::string> read_error;

        Lua.Push_Nil();

        while (!read_error.has_value() && Lua.Iterate_Over_Table(Read_Stream_Argument_Index.value())) {
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
                Function_Signature,
                parameter_name,
                read_error.value()
            );
        }

        Read_Stream_Argument_Index = Read_Stream_Argument_Index.value() + 1;

        return LuaArrayParameter(Lua, Function_Signature, parameter_name, table_array);
    }

    LuaArrayParameter Read_First_Array(std::string_view parameter_name) {
        Read_Stream_Argument_Index = 1;

        return Read_Next_Array(parameter_name);
    }

    LuaMapParameter Read_Next_Map(std::string_view parameter_name) {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        if (Read_Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", Function_Signature);
        }

        std::map<std::string, LuaVariant> table_map;
        std::optional<std::string> read_error;

        Lua.Push_Nil();

        if (!Lua.Is_Table(Read_Stream_Argument_Index.value())) {
            read_error = std::format(
                "Argument '{}' was not a table",
                parameter_name
            );
        }

        while (!read_error.has_value() && Lua.Iterate_Over_Table(Read_Stream_Argument_Index.value())) {
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
                Function_Signature,
                parameter_name,
                read_error.value()
            );
        }

        Read_Stream_Argument_Index = Read_Stream_Argument_Index.value() + 1;

        return LuaMapParameter(Lua, Function_Signature, parameter_name, table_map);
    }

    LuaMapParameter Read_First_Map(std::string_view parameter_name) {
        Read_Stream_Argument_Index = 1;

        return Read_Next_Map(parameter_name);
    }

    const std::string_view Get_Next_Read_Type() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        return Lua.Get_Lua_Type(Read_Stream_Argument_Index.value());
    }

    const std::string_view First_Read_Type() {
        Read_Stream_Argument_Index = 1;

        return Lua.Get_Lua_Type(Read_Stream_Argument_Index.value());
    }

    template<class T>
    bool Next_Read_Is() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        auto result = Lua.template Is_Type<T>(Read_Stream_Argument_Index.value());

        return result;
    }

    template<class T>
    LuaResultWithValue<T> First_Read_Is() {
        Read_Stream_Argument_Index = 1;

        return Next_Is<T>();
    }

    LuaResultWithValue<LuaVariant> Read_Next_Variant() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        if (Read_Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", Function_Signature);
        }

        auto result = Lua.Type_Read_Variant(Read_Stream_Argument_Index.value());

        Read_Stream_Argument_Index = Read_Stream_Argument_Index.value() + 1;

        return result;
    }

    LuaResultWithValue<LuaVariant> Read_First_Variant() {
        Read_Stream_Argument_Index = 1;

        return Read_Next_Variant();
    }

    template<class T>
    LuaResultWithValue<T> Read_Next() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        if (Read_Stream_Argument_Index > Count) {
            Lua.Raise_Error_Format("({}) CFunction attempted to read more arguments than were provided", Function_Signature);
        }

        auto result = Lua.Try_Read<T>(Read_Stream_Argument_Index.value());

        Read_Stream_Argument_Index = Read_Stream_Argument_Index.value() + 1;

        return result;
    }

    template<class T>
    LuaResultWithValue<T> Read_First() {
        Read_Stream_Argument_Index = 1;

        return Read_Next<T>();
    }

    void Assert_String_Parameter_Is_Valid(std::string_view name, std::string value, unsigned int max_chars){
        if (value.empty() || std::all_of(value.begin(), value.end(), ::isspace)) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' was blank",
                Function_Signature,
                name,
                max_chars
            );
        }

        if (value.length() > max_chars) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' was too long, should be at most {} characters long. Value: {}",
                Function_Signature,
                name,
                max_chars,
                value
            );
        }
    }
private:
    const LuaEngine& Lua;
    std::optional<bool> Stream_Is_Valid;
    std::optional<int> Stream_Argument_Index;

    std::optional<int> Read_Stream_Argument_Index;
};
