#include "function.h"
#include "technotypejsonreference.h"
#include "typeconverter.h"

TO_JSON(TechnoTypeClassJsonReference)
{
    CONVERT_TD_FIELD_TO_JSON(Kind);

    // Only set the Instance if Kind is not NONE
    if (p.Kind != KIND_NONE) {
        FIELD_TO_JSON(Instance);
    }
}

FROM_JSON(TechnoTypeClassJsonReference)
{
    PARSE_TD_FIELD_FROM_JSON(TechnoTypeClassJsonReference, Kind, KindType);

    // Instance is an optional value
    if (j.contains(NAMEOF(Instance))) {
        FIELD_FROM_JSON(Instance);
    }
}
