if type(__CNC_API) == "nil" or (__CNC_API.UI) == "nil" then
  error("nco.TiberianDawn.UI failed to init, required C++ backend not loaded: -- tiberiandawn/lua/ui_luaapi.h")
end

_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

_G.TiberianDawn.UI = _G.TiberianDawn.UI and _G.TiberianDawn.UI or {
  __cpp_source = __CNC_API.UI.__cpp_source,
  __name = __CNC_API.UI.__name,

  --[[
    Show a popup to the player. Supported types: 'OK'

    Supports printf style formatting.
  ]]
  showPopup = function(popupType, message, ...)
    if type(popupType) ~= "string" then
      popupType = "OK"
    end

    popupType = string.upper(popupType)

    local formatted_message = string.format(message, ...)

    if popupType == "OK" then
      __CNC_API.UI.popupOk(formatted_message);
    else
      error(string.format("Invalid popup type: {}", popupType))
    end
  end
}

return _G.TiberianDawn.UI
