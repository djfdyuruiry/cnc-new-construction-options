---@param action function
local function skipIfNotPresent(value, action)
  if type(value) == "nil" then
    action()
  end
end

---@param ... type
local function isType(...)
  local validTypes = {...}
  local typesString = ""

  for i, typeString in ipairs(validTypes) do
    if i == 1 then
      typeString = typeString .. "|"
    end

    typesString = typesString .. typeString
  end

  ---@param functionName string
  ---@param argumentName string
  ---@param value any
  return function(functionName, argumentName, value)
    local typeIsValid = false

    for _, typeString in ipairs(validTypes) do
      if type(value) == typeString then
        typeIsValid = true
      end
    end

    assert(
      typeIsValid,
      string.format("%s: argument %s must be of type(s) %s", functionName, argumentName, typesString)
    )
  end
end

---@param functionName string
---@param argumentName string
---@param value any
local function isNotNil(functionName, argumentName, value)
  return assert(
    value ~= nil,
    string.format("%s: argument %s must not be nil", functionName, argumentName)
  )
end

---@param functionName string
---@param argumentName string
---@param value string
local function isNotBlank(functionName, argumentName, value)
  if type(value) == "string" then
    -- Check if the string contains non-whitespace characters
    local _, count = value:gsub("%S", "")

    assert(
      count > 0,
      string.format("%s: argument %s must not be blank", functionName, argumentName)
    )
    return
  end

  error(
    string.format(
      "%s: invalid 'isNotBlank' for argument %s (must be of type: string)",
      functionName,
      argumentName
    )
  )
end

---@param functionName string
---@param argumentName string
---@param value table|string
local function isNotEmpty(functionName, argumentName, value)
  if type(value) == "table" then
    assert(
      next(value) ~= nil,
      string.format("%s: argument %s must not be empty", functionName, argumentName)
    )
    return
  elseif type(value) == "string" then
    assert(
      (#value) > 0,
      string.format("%s: argument %s must not be empty", functionName, argumentName)
    )
    return
  end

  error(
    string.format(
      "%s: invalid 'isNotEmpty' for argument %s (must be of type: table OR string)",
      functionName,
      argumentName
    )
  )
end

---@param functionName string
---@param argumentsMap { [string]: any[] }
---@param callingSelf boolean?
local function validateCall(functionName, argumentsMap, callingSelf)
  if not callingSelf then
    validateCall("validateCall", {
      functionName = { functionName, isType("string"), isNotBlank },
      argumentsMap = { argumentsMap, isType("table"), isNotEmpty }
    }, true)
  end

  for argumentName, validators in pairs(argumentsMap) do
    local argValue
    local stopProcessing = false

    for i = 1, #validators do
      local validatorOrArgValue = validators[i]

      if i == 1 then
        argValue = validatorOrArgValue
      elseif not stopProcessing then
        local validator = validatorOrArgValue

        if type(validator) ~= "function" then
          error("callValidator: validators must be functions")
        end

        if validator == skipIfNotPresent then
          validator(argValue, function ()
            stopProcessing = true
          end)
        else
          validator(functionName, argumentName, argValue)
        end
      end
    end
  end
end

return {
  Validators = {
    skipIfNotPresent = skipIfNotPresent,
    isType = isType,
    isNotNil = isNotNil,
    isNotBlank = isNotBlank,
    isNotEmpty = isNotEmpty
  },
  validateCall = validateCall
}
