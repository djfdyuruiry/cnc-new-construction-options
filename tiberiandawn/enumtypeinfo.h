#pragma once

#include "common/twowaymap.h"

#include "defines.h"
#include "door.h"
#include "teamtype.h"
#include "typevariants.h"

/**
 * Stores metadata about an enum type in Tiberian Dawn. Used to filter which values are exposed
 * in the Rule engine, Lua engine and INI rules. Patches enum names that don't match their INI
 * names (e.x. a Guard Tower is STRUCT_GTOWER, but is "GTWR" in INI files)
 */
template <SupportedByTdTypeConverter T>
class EnumTypeInfo final
{
public:
    const std::string Prefix;
    const T MinimumToInclude;
    const T MaximumToInclude;
    const TwoWayMap<T, std::string> PatchTable;
    const std::vector<T> Excluded;

    /**
     * Certain enum types are actually a full range of integers, with
     * 'markers' at set values. Setting this to true will allow code
     * to accept all values in the range when going to/from strings.
     *
     * These are formatted by TdTypeConverter to have the string name `${ENUM_PREFIX}_${VALUE}`
     * - so for example `DirType a = (DirType)24` would be written as `DIR_24`.
     */
    const bool AllowNonEnumValuesInRange;

    EnumTypeInfo(
        const std::string_view& prefix,
        const T& minimum_to_include,
        const T& maximum_to_include,
        const TwoWayMap<T, std::string>& patch_table = {},
        const std::vector<T>& excluded = {},
        const bool allow_non_enum_values_in_range = false
    ) : Prefix(prefix),
        MinimumToInclude(minimum_to_include),
        MaximumToInclude(maximum_to_include),
        PatchTable(patch_table),
        Excluded(excluded),
        AllowNonEnumValuesInRange(allow_non_enum_values_in_range) {}

    std::string Strip_Prefix(const std::string& subject) const
    {
        if (!subject.starts_with(Prefix)) {
            return subject;
        }

        return subject.substr(Prefix.size());
    }

    std::string Strip_Prefix(const std::string_view& subject) const
    {
        return Strip_Prefix(std::string(subject));
    }

    bool Is_Excluded(const T& instance) const
    {
        if (instance < MinimumToInclude || instance > MaximumToInclude) {
            return true;
        }

        return std::ranges::contains(Excluded, instance);
    }

    std::optional<std::string> Get_Patch_String(const T& instance) const
    {
        auto result = PatchTable[instance];

        if (!result.has_value()) {
            return std::nullopt;
        }

        return std::string(*result);
    }

    std::optional<T> Get_Patch_Instance(const std::string& subject) const
    {
        auto result = PatchTable[subject];

        if (!result.has_value()) {
            return std::nullopt;
        }

        return *result;
    }
};

// Allows template type EnumTypeInfo to be stored in stl container (see typevariants.h)
using EnumTypeInfoVariant = std::variant<
    TD_ENUMS_FORMAT(EnumTypeInfo<, > MACRO_COMMA,>)
>;
