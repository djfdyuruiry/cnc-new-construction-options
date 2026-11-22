local TdApiModule = require("nco.TiberianDawn.lib.TdApiModule")

-- TODO: Put music track names in INI settings

---@alias MusicTrack "AIRSTRIKE" | "80MX" | "CHRG" | "CREP" | "DRIL" | "DRON" | "FIST" | "RECON" | "VOICE" | "HEAVYG" | "J1" | "JDI_V2" | "RADIO" | "RAIN" | "AOI" | "CCTHANG" | "DIE" | "FWP" | "IND" | "IND2" | "JUSTDOIT" | "LINEFIRE" | "MARCH" | "MECHMAN" | "NOMERCY" | "OTP" | "PRP" | "ROUT" | "HEART" | "STOPTHEM" | "TROUBLE" | "WARFARE" | "BFEARED" | "IAM" | "VALKYRIE"

--[[
  API for controlling the in-game music track.
]]
---@class Music : ApiModule
---@field play fun(trackName: MusicTrack): nil
---@field stop fun(): nil
---@field getTrack fun(): MusicTrack?
---@field availableTracks fun(): MusicTrack[]
---@field getDisplayName fun(trackName: MusicTrack): string?

---@type Music
_G.Music = TdApiModule({
  name = "Music",
  cppSource = "tiberiandawn/lua/music_luaapi.h",
  builder = function(cppApi)
    return {
      play = cppApi.play,
      stop = cppApi.stop,
      getTrack = cppApi.getTrack,
      availableTracks = cppApi.availableTracks,
      getDisplayName = cppApi.getDisplayName,
    }
  end,
})

return _G.Music
