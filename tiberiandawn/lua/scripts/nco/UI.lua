_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {}

-- See: tiberiandawn/lua/ui_luaapi.h
_G.TiberianDawn.UI = _G.TiberianDawn.UI and _G.TiberianDawn.UI or {
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
