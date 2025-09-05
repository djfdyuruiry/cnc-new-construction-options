local function skipIfNotPresent(value, action)
  if type(value) == "nil" then
    action()
  end
end

local function isType(typeString)
  return function(functionName, argumentName, value)
    return assert(
      string.format("%s: argument %s must be of type %s", functionName, argumentName, typeString),
      type(value) == typeString
    )
  end
end

local function isNotNil(functionName, argumentName, value)
  return assert(
    string.format("%s: argument %s must not be nil", functionName, argumentName),
    value ~= nil
  )
end

local function isNotBlank(functionName, argumentName, value)
  if type(value) == "string" then
    -- Check if the string contains non-whitespace characters
    local _, count = value:gsub("%S", "")

    assert(
      string.format("%s: argument %s must not be blank", functionName, argumentName),
      count > 0
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
  if type(value) == "table" or type(value) == "string" then
    assert(
      string.format("%s: argument %s must not be empty", functionName, argumentName),
      (#value) > 0
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

    print("arg", argumentName)

    for i, validatorOrArgValue in ipairs(validators) do
      if i == 1 then
        print("value", validatorOrArgValue)

        argValue = validatorOrArgValue
      elseif not stopProcessing then
        local validator = validatorOrArgValue

        if type(validator) ~= "function" then
          error("callValidator: validators must be functions")
        end

        if validator == skipIfNotPresent then
          validator(argValue, function ()
            print("stopProcessing", i - 1)
            stopProcessing = true
          end)
        else
          print("validator", i - 1)
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
