local TypeValidator = require("nco.lib.TypeValidator")

local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

local isNotBlank = TypeValidator.Validators.isNotBlank
local isType = TypeValidator.Validators.isType

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
      TypeValidator.validateCall("showPopup", {
        popupType = {popupType, isType("string"), isNotBlank},
        message = {message, isType("string"), isNotBlank}
      })

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
_G.UI = TdApiModule({
  name = "UI",
  cppSource = "tiberiandawn/lua/ui_luaapi.h",
  builder = builder
})

return _G.UI
