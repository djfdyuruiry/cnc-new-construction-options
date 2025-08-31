#pragma once

#include <functional>
#include <memory>
#include <string>
#include <variant>

#include <lua.hpp>
#include <LuaBridge/LuaBridge.h>

#include "logger.h"

class LuaEngine {
public:
  using LuaType = std::variant<std::string_view, double, int>;

  // for storing persistent Lua state globally
  static const LuaEngine Global;

  LuaEngine();

  void With_State(std::function<void(lua_State*)> actions);

  template<class T>
  T With_State(std::function<T(lua_State*)> actions)
  {
    return actions(Runtime.get());
  }

  bool Exec(std::string_view script);
  bool Exec_File(std::string_view file_path);

  template<class T>
  std::optional<T> Try_Read(int stack_index = -1) {
    return With_State<std::optional<T>>([&stack_index](auto L) {
      auto type = lua_type(L, stack_index);

      if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
        if (type != LUA_TNUMBER) {
          CNC_LOG_WARN("Failed to read int/number from stack index {} due to type mismatch, actual type: {}", stack_index, type);
          return std::optional<T>();
        }
      }

      if constexpr (std::is_same_v<T, int>) {
        return std::make_optional(
          lua_tointeger(L, stack_index)
        );
      } else if constexpr (std::is_same_v<T, double>) {
        return std::make_optional(
          lua_tonumber(L, stack_index)
        );
      } else if constexpr (std::is_same_v<T, std::string_view>) {
        if (type != LUA_TSTRING) {
          CNC_LOG_WARN("Failed to read string from stack index {} due to type mismatch, actual type: {}", stack_index, type);
          return std::optional<std::string_view>();
        }

        return std::make_optional(
          std::string_view(lua_tostring(L, stack_index))
        );
      }

      CNC_LOG_FATAL("Attempted to read unsupported Lua type");
    });
  }

  template<class T>
  std::optional<T> Try_Read(std::string_view expression) {
    if (!Exec(std::format("return {}", expression))) {
      return std::optional<T>();
    }

    return Try_Read<T>();
  }

  luabridge::Namespace Bridge();
private:
  std::shared_ptr<lua_State> Runtime;
};
