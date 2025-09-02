(function ()
  local contains = function (tbl, value)
      for _, v in ipairs(tbl) do
          if v == value then
              return true
          end
      end
      return false
  end

  local rulesMeta = {
    __index = function(_, section)
      if type(section) == "string" then
          return setmetatable({}, {
              __index = function(_, key)
                  if type(key) == "string" then
                      -- see: common/lua/rules_luaapi.h
                      return function(value)
                          if value ~= nil then
                              __CNC_API.Rules.set(section, key, value)
                              return value
                          else
                              return __CNC_API.Rules.get(section, key)
                          end
                      end
                  end
              end
          })
      end
    end
  }

  -- Assign the rulesTable to _G.Rules for global access
  _G.Rules = setmetatable({}, rulesMeta)
end)()
