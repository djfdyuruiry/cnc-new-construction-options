--[[
  Handy Lua utility functions for working with:

    - strings
    - tables
]]

local TypeValidator = require("nco.lib.TypeValidator")

local isType = TypeValidator.Validators.isType
local isNotBlank = TypeValidator.Validators.isNotBlank
local isNotEmpty = TypeValidator.Validators.isNotEmpty
local skipIfNotPresent = TypeValidator.Validators.skipIfNotPresent

local Utils = {}

---@param array any[]
---@param delimiter string?
---@return string
Utils.arrayToCsv = function(array, delimiter)
  TypeValidator.validateCall(
    "arrayToCsv",
    {
      array = { array, isType("table"), isNotEmpty },
      delimiter = { delimiter, skipIfNotPresent, isType("string"), isNotBlank }
    }
  )

  local stringDelimiter = type(delimiter) == "string" and delimiter or ","

  return table.concat(array, stringDelimiter)
end

--- Simple table to CSV conversion for dictionary
--- style tables.
---@param tableList { [string]: any }[]
---@param delimiter string?
---@return string
Utils.tableToCsv = function(tableList, delimiter)
  TypeValidator.validateCall(
    "tableToCsv",
    {
      tableList = { tableList, isType("table"), isNotEmpty },
      delimiter = { delimiter, skipIfNotPresent, isType("string"), isNotBlank }
    }
  )

  local stringDelimiter = type(delimiter) == "string" and delimiter or ","

  local result = ""

  -- Get the keys from the first table to use as header
  local keys = {}

  for k, _ in pairs(tableList[1]) do
    table.insert(keys, k)
  end

  -- Sort the keys table
  table.sort(keys)

  -- Build header row
  for i = 1, #keys do
    if i > 1 then
      result = result .. stringDelimiter
    end
    local sanitizedKey = tostring(keys[i])

    -- Escape instances of stringDelimiter with a leading backslash
    sanitizedKey = sanitizedKey:gsub(stringDelimiter, "\\" .. stringDelimiter)

    result = result .. sanitizedKey
  end

  result = result .. "\n"

  -- Add data rows
  for i = 1, #tableList do
    -- Get values for each key in order
    for j = 1, #keys do
      if j > 1 then
        result = result .. stringDelimiter
      end
      local sanitizedValue = tostring(tableList[i][keys[j]])

      -- Escape instances of stringDelimiter with a leading backslash
      sanitizedValue = sanitizedValue:gsub(stringDelimiter, "\\" .. stringDelimiter)

      result = result .. sanitizedValue
    end

    result = result .. "\n"
  end

  return result
end

---@param subject string
---@param delimiter string
---@return table
Utils.splitString = function(subject, delimiter)
  TypeValidator.validateCall(
    "splitString",
    {
      subject = { subject, isType("string"), isNotBlank },
      delimiter = { delimiter, isType("string"), isNotBlank }
    }
  )

  local result = {}
  local currentString = ""

  if #delimiter > 1 then
    error("splitString: delimiter must be one character")
  end

  for i = 1, #subject do
    if subject:sub(i, i) == delimiter then
      table.insert(result, currentString)
      currentString = ""
    else
      currentString = currentString .. subject:sub(i, i)
    end
  end

  if currentString ~= "" then
    table.insert(result, currentString)
  end

  return result
end

return Utils
