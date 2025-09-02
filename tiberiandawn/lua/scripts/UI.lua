_G.TiberianDawn = _G.TiberianDawn and _G.TiberianDawn or {
  Game = {},
  Messages = {},
  UI = {}
}

_G.TiberianDawn.UI.showPopup = function(popupType, message, ...)
  if type(popupType) ~= "string" then
    popupType = "OK"
  end

  popupType = string.upper(popupType)

  local formatted_message = string.format(message, ...)

  if popupType == "OK" then
    -- See: tiberiandawn/lua/game_luaapi.h
    __CNC_API.UI.popupOk(formatted_message);
  else
    error(string.format("Invalid popup type: {}", popupType))
  end
end
