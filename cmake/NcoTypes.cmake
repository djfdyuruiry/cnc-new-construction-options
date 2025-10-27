##
# Type properties code generator, allows keeping game properties metadata in one place for a single source of truth.
#
# Params (mandatory):
#
#   * TYPES_PATH - Path to a directory that contains .json files, each describing a <type>.ini section (unit.ini, infantry.ini etc)
#   * TYPES_TEMPLATE_PATH - Directory where templates can be found, and will be rendered to (.in files mentioned in .json file template field)
#
# This script reads all JSON files in ${TYPES_PATH}, one JSON file per <type>.ini file. The properties in each file
# are used to generate C++ code for the Read_INI and Read_Rules methods of the target type.
#
# Expected structure of JSON files is defined in the schema file 'nco-type-json-schema.json', use this in your files
# to validate them in VSCode etc.:
#
#   ```json
#   {
#     "$schema": "../../cmake/nco-type-json-schema.json",
#     "name": "MyType"
#   }
#   ```
#
# Generates two separate blocks of C++ code per type and provides them as variables to the above templates:
#
#   * LOAD_RULES_CODE - Methods calls that load the type properties from a INI file
#   * READ_RULES_CODE - Method calls that read the type properties from the rules cache (used to update types by Lua API)
#
# All JSON files are watched for changes, automatically regenerating code before build and on configure. Adding/deleting
# JSON files is also detected automatically. The template files passed in as params are also watched.
#
##
macro(CHECK_REQUIRED_VARIABLE var_name)
  if(NOT DEFINED ${var_name})
    message(FATAL_ERROR "[NcoTypeRules] The \"${var_name}\" variable must be defined.")
  endif()
endmacro()

CHECK_REQUIRED_VARIABLE(TYPES_PATH)
CHECK_REQUIRED_VARIABLE(TYPES_TEMPLATE_PATH)

function(LoadTypeProperties _TYPE_JSON _PROP_INDEX _PROP_NAME _PROP_TYPE _REQ_CONVERTER _REQ_CSV_CONVERTER)
  string(JSON TYPE_OBJECT_JSON GET "${_TYPE_JSON}" properties "${_PROP_INDEX}")

  string(JSON PROP_NAME GET "${TYPE_OBJECT_JSON}" name)
  string(JSON PROP_TYPE GET "${TYPE_OBJECT_JSON}" type)

  string(JSON REQ_CONVERTER ERROR_VARIABLE JSON_ERROR GET "${TYPE_OBJECT_JSON}" requires_converter)

  if(NOT ${REQ_CONVERTER} STREQUAL "ON")
    # default converter flag to false
    set(REQ_CONVERTER "OFF")
  endif()

  string(JSON REQ_CSV_CONVERTER ERROR_VARIABLE JSON_ERROR GET "${TYPE_OBJECT_JSON}" requires_csv_converter)

  if(NOT ${REQ_CSV_CONVERTER} STREQUAL "ON")
    # default csv converter flag to false
    set(REQ_CSV_CONVERTER "OFF")
  endif()

  set("${_PROP_NAME}" ${PROP_NAME} PARENT_SCOPE)
  set("${_PROP_TYPE}" ${PROP_TYPE} PARENT_SCOPE)
  set("${_REQ_CONVERTER}" ${REQ_CONVERTER} PARENT_SCOPE)
  set("${_REQ_CSV_CONVERTER}" ${REQ_CSV_CONVERTER} PARENT_SCOPE)
endfunction()

function (ExtractTypeInfoFromJson _TYPE_JSON _TYPE_NAME _TEMPLATE_FILE _PROP_COUNT)
  string(JSON TYPE_NAME GET "${_TYPE_JSON}" name)
  string(JSON TEMPLATE_FILE GET "${_TYPE_JSON}" template)
  string(JSON PROP_COUNT LENGTH "${TYPE_JSON}" properties)

  MATH(EXPR PROP_COUNT "${PROP_COUNT}-1")

  message(STATUS "[NcoTypeRules] Type: ${TYPE_NAME}")

  set("${_TYPE_NAME}" ${TYPE_NAME} PARENT_SCOPE)
  set("${_TEMPLATE_FILE}" ${TEMPLATE_FILE} PARENT_SCOPE)
  set("${_PROP_COUNT}" ${PROP_COUNT} PARENT_SCOPE)
endfunction()


function(ParseTypesFilePath _TYPE_FILE _RELATIVE_TYPE_FILE)
  file(RELATIVE_PATH RELATIVE_TYPE_FILE "${TYPES_PATH}" "${_TYPE_FILE}")

  message(STATUS "[NcoTypeRules] Processing type file: ${RELATIVE_TYPE_FILE}")

  set("${_RELATIVE_TYPE_FILE}" ${RELATIVE_TYPE_FILE} PARENT_SCOPE)
endfunction()

function(SetupTypesCheckBeforeBuild)
  if(_BUILD_TIME_TYPES)
    # This prevents attempting to define this check when this file is called by
    # the custom target we define below (think of it like a recursion check).
    return()
  endif()

  message(STATUS "[NcoTypeRules] Setting up pre-build Types check")

  # Custom target ensures any type files made after configure and before build
  # are detected and this cmake file is re-ran to regen code. Current variable
  # context is inherited.
  add_custom_target(
    td_types
    ALL
    BYPRODUCTS # TODO: rendered templates
      ${TYPES_STATE_FILE}
    COMMENT "[NcoTypeRules] Checking type files for changes..."
    COMMAND
      ${CMAKE_COMMAND}
      -DTYPES_PATH=${TYPES_PATH}
      -DTYPES_TEMPLATE_PATH=${TYPES_TEMPLATE_PATH}
      -DTYPES_STATE_FILE=${TYPES_STATE_FILE}
      -D_BUILD_TIME_TYPES=TRUE
      -P "${CMAKE_CURRENT_LIST_FILE}"
  )
endfunction()

function(WatchFileForChanges _FILE)
  # ensures the file is seen as a dependency of the configure phase
  set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS ${_FILE}
  )
endfunction()

function(ScanForTypeFiles _TYPE_STATE_FILE _TYPES_FILES _TYPES_HASH _FILES_HAVE_CHANGED)
  message(STATUS "[NcoTypeRules] Scanning type files...")

  file(GLOB TYPES_FILES LIST_DIRECTORIES false "${TYPES_PATH}/*.json")

  set(TYPES_HASH "")

  foreach(TYPE_FILE ${TYPES_FILES})
    # add hash of file contents to overall rules hash
    file(SHA256 "${TYPE_FILE}" FILE_HASH)
    string(SHA256 TYPES_HASH "${TYPES_HASH}${FILE_HASH}")

    WatchFileForChanges("${TYPE_FILE}")
  endforeach()

  # TODO: ensure templates are included in hash so code is generated if they change

  # If a previous hash was calculated, and does not match the
  # hash we just calculated, then flag that files have changed.
  set(FILES_HAVE_CHANGED true)

  if(EXISTS "${_TYPE_STATE_FILE}")
    file(READ "${_TYPE_STATE_FILE}" OLD_RULES_HASH)
    if(OLD_RULES_HASH STREQUAL "${TYPES_HASH}")
        set(FILES_HAVE_CHANGED false)
    endif()
  endif()

  set("${_TYPES_FILES}" ${TYPES_FILES} PARENT_SCOPE)
  set("${_TYPES_HASH}" ${TYPES_FILES} PARENT_SCOPE)
  set("${_FILES_HAVE_CHANGED}" ${FILES_HAVE_CHANGED} PARENT_SCOPE)
endfunction()

function(Main)
  if(NOT DEFINED "TYPE_STATE_FILE")
    set(TYPES_STATE_FILE "${CMAKE_BINARY_DIR}/td-type-rules-hash")
  endif()

  ScanForTypeFiles("${TYPES_STATE_FILE}" TYPE_FILES TYPES_HASH FILES_HAVE_CHANGED)

  if(NOT FILES_HAVE_CHANGED)
    message(STATUS "[NcoTypeRules] No Type changes detected")
    SetupTypesCheckBeforeBuild()

    return()
  endif()

  message(STATUS "[NcoTypeRules] Generating Rules code...")

  foreach(TYPE_FILE ${TYPE_FILES})
    ParseTypesFilePath("${TYPE_FILE}" RELATIVE_TYPE_FILE)

    # load properties from JSON file
    file(READ ${TYPE_FILE} TYPE_JSON)

    ExtractTypeInfoFromJson("${TYPE_JSON}" TYPE_NAME TEMPLATE_FILE PROP_COUNT)

    message(STATUS "[NcoTypeRules] Generating code for type: ${TYPE_NAME}")
    SET(LOAD_RULES_CODE "")
    SET(READ_RULES_CODE "")

    foreach(PROP_INDEX RANGE ${PROP_COUNT})
      if(${PROP_INDEX} GREATER 0)
        string(APPEND LOAD_RULES_CODE "\n        ")
        string(APPEND READ_RULES_CODE "\n        ")
      endif()

      LoadTypeProperties("${TYPE_JSON}" "${PROP_INDEX}" PROP_NAME PROP_TYPE REQ_CONVERTER REQ_CSV_CONVERTER)

      message(STATUS "[NcoTypeRules] Processing property #${PROP_INDEX}: ${PROP_NAME} (type=${PROP_TYPE})")

      if(${REQ_CONVERTER} STREQUAL "OFF" AND ${REQ_CSV_CONVERTER} STREQUAL "OFF")
        string(APPEND LOAD_RULES_CODE ".Load_${PROP_TYPE}_Var(${PROP_NAME})")
        string(APPEND READ_RULES_CODE ".Read_${PROP_TYPE}_Var(${PROP_NAME})")
      endif()

      if(${REQ_CONVERTER} STREQUAL "ON")
        string(APPEND LOAD_RULES_CODE ".Load_With_TdConverter(${PROP_TYPE}, ${PROP_NAME})")
        string(APPEND READ_RULES_CODE ".Read_With_TdConverter(${PROP_TYPE}, ${PROP_NAME})")
      endif()

      if(${REQ_CSV_CONVERTER} STREQUAL "ON")
        string(APPEND LOAD_RULES_CODE ".Load_Csv_With_TdConverter(${PROP_TYPE}, ${PROP_NAME})")
        string(APPEND READ_RULES_CODE ".Read_Csv_With_TdConverter(${PROP_TYPE}, ${PROP_NAME})")
      endif()
    endforeach()

    cmake_path(APPEND TYPES_TEMPLATE_PATH ${TEMPLATE_FILE} OUTPUT_VARIABLE TEMPLATE_PATH)
    SET(TEMPLATE_OUTPUT_PATH ${TEMPLATE_PATH})
    cmake_path(REMOVE_EXTENSION TEMPLATE_OUTPUT_PATH LAST_ONLY)

    message(STATUS "[NcoTypeRules] Rendering template: ${TEMPLATE_PATH} -> ${TEMPLATE_OUTPUT_PATH}")

    configure_file(${TEMPLATE_PATH} ${TEMPLATE_OUTPUT_PATH} @ONLY)

    list(APPEND ${NCO_SRC_LIST} ${TEMPLATE_OUTPUT_PATH})

    WatchFileForChanges(${TEMPLATE_PATH})
  endforeach()

  # save rules hash to file, used in ScanForTypeFiles() to detect if changes happen
  message(STATUS "[NcoTypeRules] Saving Types state")
  file(WRITE "${TYPES_STATE_FILE}" "${TYPES_HASH}")

  SetupTypesCheckBeforeBuild()
endfunction()

Main()
