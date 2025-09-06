local ApiModule = require("nco.lib.ApiModule")

---@alias PopupType "OK"

--[[
  API that allows controlling the Tiberian Dawn user interface (menus, popups etc.)

  Note: Popup messages pause the game until the user interacts with the dialog
]]
---@class UI : ApiModule
---@field showPopup fun(type: PopupType, message: string, ...)
---@field showOkPopup fun(message: string, ...)

---@returns UI
local function builder(cppApi)
  local ui = {
    ---@param popupType PopupType
    ---@param message string
    showPopup = function(popupType, message, ...)
      popupType = string.upper(popupType)

      local formatted_message = string.format(message, ...)

      if popupType == "OK" then
        cppApi.popupOk(formatted_message);
      else
        error(string.format("Invalid popup type: {}", popupType))
      end
    end
  }

  ui.showOkPopup = function(message, ...)
    ui.showPopup("OK", message, ...)
  end

  return ui
end

---@type UI
local UI = ApiModule({
  modulePath = { "TiberianDawn", "UI" },
  cppApi = "UI",
  cppSource = "tiberiandawn/lua/ui_luaapi.h",
  builder = builder
})

return UI

