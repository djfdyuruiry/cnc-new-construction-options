#pragma once

#pragma region IniRuleContext Macros

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Read_With_TdConverter(TYPE, VAR) \
    Get_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](const auto& v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Read_Csv_With_TdConverter(TYPE, VAR) \
    Get_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, [&](auto v) { VAR = std::move(v); })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a non-trivial type
#define Load_With_TdConverter(TYPE, VAR) \
    Load_With_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](const auto& v) { VAR = v; })

// IniRuleContext macro 'method' for loading types that are converted from string representation to a list of non-trivial type instances
#define Load_Csv_With_TdConverter(TYPE, VAR) \
    Load_With_Csv_Converter_Callback<TYPE, TdTypeConverter>(#VAR, VAR, [&](auto v) { VAR = std::move(v); })

#pragma endregion

#pragma region Target<-Ptr->Target Macros

// Build target value for given pointer
#define OBJECT_PTR_TO_TARGET(PTR) static_cast<TARGET>(PTR == nullptr ? 0 : PTR->As_Target())

// Build techno target value for given pointer
#define TECHNO_TYPE_PTR_TO_TARGET(PTR) static_cast<TARGET>(PTR == nullptr ? 0 : TechnoType_To_Target(PTR))

// Convert target to a type compatible with a TYPE pointer address
#define TARGET_TO_PTR_WITH_TYPE(TARGET, TYPE) reinterpret_cast<TYPE*>(static_cast<intptr_t>(TARGET))

// Convert target to a type compatible with a ObjectClass pointer address
#define OBJECT_TARGET_TO_PTR(TARGET) TARGET_TO_PTR_WITH_TYPE(TARGET, ObjectClass)

// Convert target to a type compatible with a TechnoTypeClass pointer address
#define TECHNO_TYPE_TARGET_TO_PTR(TARGET) TARGET_TO_PTR_WITH_TYPE(TARGET, TechnoTypeClass)

#pragma endregion

#pragma region To JSON Macros

// Store target value for ObjectTypeClass pointer in JSON field
#define OBJECT_TARGET_PTR_TO_JSON(FIELD) FIELD_VALUE_TO_JSON(FIELD, OBJECT_PTR_TO_TARGET(p.FIELD))

// Store reference value for TechnoTypeClass pointer of TYPE in JSON field
#define TECHNO_TYPE_PTR_REF_TO_JSON(FIELD) \
    FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::Techno_Type_Target_To_Json(p.FIELD))

// Store target values for array of ObjectTypeClass pointer memory addresses in JSON array
#define OBJECT_TARGET_PTR_ARRAY_TO_JSON(FIELD) \
    FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::Object_Target_Array_To_Json(p.FIELD, std::size(p.FIELD)));

// Store target values for array of TechnoTypeClass pointer memory addresses in JSON array
#define TECHNO_TYPE_PTR_REF_ARRAY_TO_JSON(FIELD) \
    FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::Techno_Type_Target_Array_To_Json(p.FIELD, std::size(p.FIELD)));

// Convert TD type field to string and store in JSON object, actual field value can be any expression
// (e.g. fetch Type enum value from pointer object)
#define CONVERT_TD_FIELD_VALUE_TO_JSON(FIELD, VALUE) \
    CONVERT_FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::To_String, VALUE)

// Convert TD type field to string and store in JSON object
#define CONVERT_TD_FIELD_TO_JSON(FIELD) CONVERT_FIELD_TO_JSON(FIELD, TdTypeConverter::To_String)

// Convert TD type array to csv string and store in JSON object
#define CONVERT_TD_ARRAY_FIELD_TO_JSON(FIELD, TYPE) \
    FIELD_VALUE_TO_JSON(FIELD, TdTypeConverter::To_Csv_String(p.FIELD, std::size(p.FIELD)))

#pragma endregion

#pragma region From JSON Macros

// Load target value from JSON into pointer memory address
#define TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, TYPE) p.FIELD = TARGET_TO_PTR_WITH_TYPE(j.at(#FIELD).get<TARGET>(), TYPE)

// Load target value from JSON into pointer memory address
#define TARGET_CONST_PTR_FROM_JSON_WITH_TYPE(FIELD, TYPE) \
    ((TYPE const*&)p.FIELD) = TARGET_TO_PTR_WITH_TYPE(j.at(#FIELD).get<TARGET>(), TYPE)

// Load target value for ObjectTypeClass into pointer memory address
#define OBJECT_TARGET_PTR_FROM_JSON(FIELD) TARGET_PTR_FROM_JSON_WITH_TYPE(FIELD, ObjectClass)

// Load target value for ref to TechnoTypeClass of TYPE into pointer memory address
#define TECHNO_TYPE_TARGET_PTR_FROM_REF_JSON_WITH_TYPE(CLASS, FIELD, TYPE) \
    TdTypeConverter::Techno_Type_Target_From_Json<TYPE>(j.at(#FIELD), #CLASS, #FIELD, p.FIELD)

// Load target value for ref to TechnoTypeClass into pointer memory address
#define TECHNO_TYPE_TARGET_PTR_FROM_REF_JSON(CLASS, FIELD) \
    TECHNO_TYPE_TARGET_PTR_FROM_REF_JSON_WITH_TYPE(CLASS, FIELD, TechnoTypeClass)

// Load target value for ref to TechnoTypeClass of TYPE into const pointer memory address
#define TECHNO_TYPE_TARGET_CONST_PTR_FROM_REF_JSON_WITH_TYPE(CLASS, FIELD, TYPE) \
    TdTypeConverter::Techno_Type_Target_From_Json<TYPE>(j.at(#FIELD), #CLASS, #FIELD, const_cast<TYPE*&>(p.FIELD))

// Load target value for ref to TechnoTypeClass into const pointer memory address
#define TECHNO_TYPE_TARGET_CONST_PTR_FROM_REF_JSON(CLASS, FIELD) \
    TECHNO_TYPE_TARGET_CONST_PTR_FROM_REF_JSON_WITH_TYPE(CLASS, FIELD, TechnoTypeClass)

// Load target values for array of ObjectTypeClass pointer memory addresses
#define OBJECT_TARGET_PTR_ARRAY_FROM_JSON(CLASS, FIELD, TYPE) \
    TdTypeConverter::Object_Target_Array_From_Json<TYPE>( \
        j.at(#FIELD), #CLASS, #FIELD, p.FIELD, std::size(p.FIELD) \
    )

// Load target values for array of refs to TechnoTypeClass of TYPE pointer memory addresses
#define TECHNO_TYPE_TARGET_PTR_ARRAY_FROM_JSON(CLASS, FIELD, TYPE) \
    TdTypeConverter::Techno_Type_Target_Array_From_Json<TYPE>( \
        j.at(#FIELD), #CLASS, #FIELD, p.FIELD, std::size(p.FIELD) \
    )

// Parse TD type field from JSON string
#define PARSE_TD_FIELD_FROM_JSON(CLASS, FIELD, TYPE) \
    TdTypeConverter::Load_Field_From_Json<TYPE>(j, #CLASS, #FIELD, [&](const auto& v) { p.FIELD = v; })

// Parse TD type field from JSON string
#define PARSE_TD_ARRAY_FIELD_FROM_JSON(CLASS, FIELD, TYPE) \
    TdTypeConverter::Load_Csv_Field_From_Json<TYPE>(j, #CLASS, #FIELD, std::size(p.FIELD), p.FIELD)

#pragma endregion
