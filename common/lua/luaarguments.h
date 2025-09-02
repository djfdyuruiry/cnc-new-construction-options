#pragma once

#include <format>
#include <functional>
#include <optional>
#include <string>

#include <lua.hpp>

#include "../logger.h"
#include "luaengine.h"
#include "luaresult.h"

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
            Lua.Raise_Error("CFunction attempted to validate more arguments than were expected");
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
            Lua.Raise_Error("CFunction attempted to validate more arguments than were expected");
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
            Lua.Raise_Error("CFunction attempted to validate more arguments than were expected");
        }

        Stream_Is_Valid = !Lua.Is_None(Stream_Argument_Index.value());
        Stream_Argument_Index = Stream_Argument_Index.value() + 1;

        return *this;
    }

    LuaArguments& First_Argument_Is_Not_None() {
        Stream_Argument_Index = 1;

        return Next_Argument_Is_Not_None();
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

    template<class T>
    LuaResultWithValue<T> Read_Next() {
        if (!Read_Stream_Argument_Index.has_value()) {
            Read_Stream_Argument_Index = 1;
        }

        if (Read_Stream_Argument_Index > Count) {
            Lua.Raise_Error("Attempted to read more arguments than were provided.");
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
private:
    const LuaEngine& Lua;
    std::optional<bool> Stream_Is_Valid;
    std::optional<int> Stream_Argument_Index;

    std::optional<int> Read_Stream_Argument_Index;
};
