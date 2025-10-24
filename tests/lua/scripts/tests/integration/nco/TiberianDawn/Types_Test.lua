-- test getting and setting every property for an infantry instance
for _, p in pairs(Types.Infantry.getPropertyNames()) do
  local before = Types.Infantry.E1[p]

  Types.Infantry.E1[p] = Types.Infantry.E1[p]

  local after = Types.Infantry.E1[p]

  if before ~= after then
    error(
      string.format(
        "Infantry type API error for property %s - set call returned incorrect value (Expected: %s - Actual: %s)",
        p,
        tostring(before),
        tostring(after)
      )
    )
  end
end
