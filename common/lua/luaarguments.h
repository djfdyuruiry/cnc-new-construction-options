#pragma once

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "logger.h"

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
        const std::string_view function_signature,
        const std::string_view parameter,
        std::map<std::string, LuaVariant> data
    ): Lua(lua), FunctionSignature(function_signature), Parameter(parameter), Data(std::move(data)) {}

    template<LuaVariantCompatible T>
    LuaMapParameter& With_Key(std::string key)
    {
        if (!Data.contains(key)) {
            Lua.Raise_Error_Format(
                "({}) Parameter '{}' didn't contain expected key: {}",
                FunctionSignature,
                Parameter,
                key
            );
        }

        if (!std::holds_alternative<T>(Data[key])) {
            auto expected_lua_type = LUA_TNONE;

            if constexpr (std::is_same_v<T, std::string>) {
                expected_lua_type = LUA_TSTRING;
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, double>) {
                expected_lua_type = LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                expected_lua_type = LUA_TBOOLEAN;
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

    template<LuaVariantCompatible T>
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
        const std::string_view function_signature,
        const std::string_view parameter,
        std::vector<LuaVariant> data
    ): Lua(lua), FunctionSignature(function_signature), Parameter(parameter), Data(std::move(data)) {}

    std::size_t Get_Size() const;

    template<LuaVariantCompatible T>
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

        if (!std::holds_alternative<T>(Data[idx])) {
            auto expected_lua_type = LUA_TNONE;

            if constexpr (std::is_same_v<T, std::string>) {
                expected_lua_type = LUA_TSTRING;
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                expected_lua_type = LUA_TNUMBER;
            } else if constexpr (std::is_same_v<T, bool>) {
                expected_lua_type = LUA_TBOOLEAN;
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

    template<LuaVariantCompatible T>
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
        Count(lua.Get_Stack_Count()),
        FunctionSignature(std::move(function_signature)),
        Lua(lua) {}

    // Fluent assert stream methods

    LuaArguments& Count_Is(int expected);

    template<LuaVariantCompatible T>
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

        StreamIsValid = Lua.Is_Type<T>(StreamArgumentIndex.value());
        StreamArgumentIndex = StreamArgumentIndex.value() + 1;

        return *this;
    }

    template<LuaVariantCompatible T>
    LuaArguments& First_Argument_Is()
    {
        StreamArgumentIndex = 1;

        return Next_Argument_Is<T>();
    }

    LuaArguments& Next_Argument_Is_Not_Nil();

    LuaArguments& First_Argument_Is_Not_Nil();

    LuaArguments& Next_Argument_Is_Not_None();

    LuaArguments& First_Argument_Is_Not_None();

    LuaArguments& Next_Argument_Is_Table();

    LuaArguments& First_Argument_Is_Table();

    bool Assert();

    // Fluent read stream methods

    LuaArrayParameter Read_Next_Array(std::string_view parameter_name);

    LuaArrayParameter Read_First_Array(std::string_view parameter_name);

    LuaMapParameter Read_Next_Map(std::string_view parameter_name);

    LuaMapParameter Read_First_Map(std::string_view parameter_name);

    std::string_view Get_Next_Read_Type();

    std::string_view First_Read_Type();

    template<LuaVariantCompatible T>
    bool Next_Read_Is()
    {
        if (!ReadStreamArgumentIndex.has_value()) {
            ReadStreamArgumentIndex = 1;
        }

        auto result = Lua.Is_Type<T>(ReadStreamArgumentIndex.value());

        return result;
    }

    template<LuaVariantCompatible T>
    LuaResultWithValue<T> First_Read_Is()
    {
        ReadStreamArgumentIndex = 1;

        return Next_Read_Is<T>();
    }

    LuaResultWithValue<LuaVariant> Read_Next_Variant();

    LuaResultWithValue<LuaVariant> Read_First_Variant();

    template<LuaVariantCompatible T>
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

    template<LuaVariantCompatible T>
    LuaResultWithValue<T> Read_First()
    {
        ReadStreamArgumentIndex = 1;

        return Read_Next<T>();
    }

    void Assert_String_Parameter_Is_Valid(std::string_view name, const std::string& value) const;

    void Assert_String_Parameter_Is_Valid(std::string_view name, const std::string& value, unsigned int max_chars) const;
private:
    const LuaEngine& Lua;
    std::optional<bool> StreamIsValid;
    std::optional<int> StreamArgumentIndex;

    std::optional<int> ReadStreamArgumentIndex;
};
