local function skipIfNotPresent(value, action)
  if type(value) == "nil" then
    action()
  end
end

local function isType(typeString)
  return function(functionName, argumentName, value)
    assert(
      type(value) == typeString,
      string.format("%s: argument %s must be of type %s", functionName, argumentName, typeString)
    )
  end
end

local function isNotNil(functionName, argumentName, value)
  return assert(
    value ~= nil,
    string.format("%s: argument %s must not be nil", functionName, argumentName)
  )
end

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
