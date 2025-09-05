local function isType(typeString)
  return function(functionName, argumentName, value)
    return assert(
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
  if type(value) == "string" and #value > 0 then
    -- Check if the string contains non-whitespace characters
    local _, count = value:gsub("%S", "")

    assert(
      count > 0,
      string.format("%s: argument %s must not be blank", functionName, argumentName)
    )
  end

  error(
    string.format(
      "%s: invalid 'isNotBlank' for argument %s (must be of type 'string')",
      functionName,
      argumentName
    )
  )
end

local function isNotEmpty(functionName, argumentName, value)
  if type(value) == "table" or type(value) == "string" then
    assert(
      #value > 0,
      string.format("%s: argument %s must not be empty", functionName, argumentName)
    )
  end

  return false
end

local function validateCall(functionName, argumentsMap)
  validateCall("validateCall", {
    functionName = { functionName, isType("string"), isNotBlank },
    argumentsMap = { argumentsMap, isType("table"), isNotEmpty }
  })

  for argumentName, validators in pairs(argumentsMap) do
    local argValue

    for i, validatorOrArgValue in ipairs(validators) do
      if i == 1 then
        argValue = validatorOrArgValue
      end

      local validator = validatorOrArgValue

      if type(validator) ~= "function" then
        error("callValidator: validators must be functions")
      end

      validator(functionName, argumentName, argValue)
    end
  end
end

return {
  Validators = {
    isType = isType,
    isNotNil = isNotNil,
    isNotBlank = isNotBlank,
    isNotEmpty = isNotEmpty
  },
  validateCall = validateCall
}
