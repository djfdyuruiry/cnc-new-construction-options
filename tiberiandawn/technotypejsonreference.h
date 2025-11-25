#pragma once

#include "common/json.h"

#include "target.h"

/**
 * This class replaces using TARGET to serialize a pointer to a TechnoTypeClass instance
 * (or subclass instance). It ensures that the kind and instance are serialized to and from
 * strings, which decouples JSON references from enum number values (e.g. InfantryType).
 */
class TechnoTypeClassJsonReference
{
public:
    KindType Kind;
    std::string Instance;

    JSON_FUNCTIONS(TechnoTypeClassJsonReference)
};
