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
