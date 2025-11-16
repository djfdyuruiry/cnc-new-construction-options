#include "stage.h"

TO_JSON(StageClass)
{
    FIELD_TO_JSON(Stage);
    FIELD_TO_JSON(StageTimer);
    FIELD_TO_JSON(Rate);
}

FROM_JSON(StageClass)
{
    FIELD_FROM_JSON(Stage);
    FIELD_FROM_JSON(StageTimer);
    FIELD_FROM_JSON(Rate);
}
