--[[
  Simple Path class for building and querying file
  paths for the current operating system.

  Short hand join using '/' operator:

    local a = Path(...)
    local b = Path(...)

    -- outputs "<a><pathSeparator><b><pathSeparator>sub<pathSeparator>path<pathSeparator>file.txt"
    --         e.x.  "<a>/<b>/sub/path/file.txt" on *nix
    --               "<a>\<b>\sub\path\file.txt" on windows 
    local joined_path = a / b / "sub" / "path" / "file.txt"
]]

local TypeValidator = require("nco.lib.TypeValidator")

local isType = TypeValidator.Validators.isType
local isNotBlank = TypeValidator.Validators.isNotBlank

local Path

---@class Path
---@field parts string[]
---@field join fun(otherPathOrString: Path|string): Path
---@field isRelative fun(): boolean
---@field isSubPathOf fun(otherPathOrString: Path|string): boolean
---@field asRelativeSubPathOf fun(otherPathOrString: Path|string): Path
---@field getRoot fun(): Path
---@field pathString string
---@operator div(Path|string): Path

---@type function
---@param pathStringOrPath Path|string
---@param separator string
---@param isWindows boolean
---@returns Path
Path = function(pathStringOrPath, separator, isWindows)
  TypeValidator.validateCall("Path", {
    pathStringOrPath = {pathStringOrPath, isType("string", "table")},
    separator = {separator, isType("string"), isNotBlank},
    isWindows = {isWindows, isType("boolean")}
  })

  local windowsDrivePattern = "^[A-Z]:\\"

  local pathString = tostring(pathStringOrPath)

  local function getParts()
    local result = {}
    local currentPart = ""

    for i = 1, #pathString do
      local char = pathString:sub(i, i)

      if char == separator then
        if currentPart ~= "" then
          table.insert(result, currentPart)
          currentPart = ""
        end
      else
        currentPart = currentPart .. char
      end
    end

    -- Add the last part if it exists
    if currentPart ~= "" then
      table.insert(result, currentPart)
    end

    return result
  end

  local parts = getParts()

  local function marshalToPath(pathOrString)
      if type(pathOrString) == "string" then
        return Path(pathOrString, separator, isWindows)
      end

      return pathOrString
  end

  local function isRelative()
    return isWindows
      and pathString:upper():match(windowsDrivePattern) == nil
      or pathString:sub(1, 1) ~= separator
  end

  local function getRoot()
    if isRelative() then
      return parts[1]
    end

    local rootPath = isWindows
      and pathString:upper():match(windowsDrivePattern)
      or separator

    return Path(rootPath, separator, isWindows)
  end

  local function join (otherPathOrString)
    local otherPath = marshalToPath(otherPathOrString)

    if not otherPath.isRelative() then
      error(
        string.format(
          "Path passed to Path.join(...) was not relative: %s",
          otherPath.pathString
        )
      )
    end

    local resolvedPathString =
      (not isRelative() and #parts == 1)
      and pathString
      or pathString .. separator

    return Path(
      resolvedPathString .. otherPath.pathString,
      separator,
      isWindows
    )
  end

  local function isSubPathOf(otherPathOrString)
    local potentialAncestorPath = marshalToPath(otherPathOrString)

    -- If this path has fewer parts than the other path, it can't be a subpath
    if #parts < #(potentialAncestorPath.parts) then
      return false
    end

    -- Compare each part of the other path with the corresponding part of this path
    for i = 1, #(potentialAncestorPath.parts) do
      if parts[i] ~= potentialAncestorPath.parts[i] then
        -- some part of the potential subpath mismatches
        return false
      end
    end

    return true
  end

  local function asRelativeSubPathOf(otherPathOrString)
    local otherPath = marshalToPath(otherPathOrString)

    if not isSubPathOf(otherPath) then
      error(
        string.format(
          "Path passed to Path.asRelativeSubPathOf(...), '%s', was not an ancestor of the current path: %s",
          otherPathOrString,
          pathString
        )
      )
    end

    local subPathParts = {}

    for i = ((#otherPath.parts) + 1), #parts do
      table.insert(subPathParts, parts[i])
    end

    local subPathString = ""

    for i, subPathPart in ipairs(subPathParts) do
      if i == 1 then
        subPathString = subPathPart
      else
        subPathString = subPathString .. separator .. subPathPart
      end
    end

    return Path(subPathString, separator, isWindows)
  end

  return setmetatable(
    {
      parts = parts,
      join = join,
      isRelative = isRelative,
      isSubPathOf = isSubPathOf,
      asRelativeSubPathOf = asRelativeSubPathOf,
      getRoot = getRoot,
      pathString = pathString
    },
    {
      ---@return Path
      __div = function (firstPath, secondPath)
        return firstPath.join(secondPath)
      end,
      __tostring = function()
        return pathString
      end
    }
  )
end

return Path;
