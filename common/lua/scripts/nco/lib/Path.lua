local Path

Path = function(pathStringOrPath, separator, isWindows)
  local windowsDrivePattern = "^[A-Z]:\\"

   local pathString = type(pathStringOrPath) == "table"
    and pathStringOrPath.pathString
    or pathStringOrPath

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

  return
  setmetatable(
    {
      parts = parts,
      join = join,
      isRelative = isRelative,
      getRoot = getRoot,
      pathString = pathString
    },
    {
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
