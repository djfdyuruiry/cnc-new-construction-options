##
# Rules code generator, allows keeping game rule metadata in one place
# for a single source of truth.
#
# Params (mandatory):
#
#   * RULES_PATH - Path to a directory that contains .json files, each describing a rules.ini section
#   * RULES_KEYS_TEMPLATE_PATH - Template for generated .h file that defines consts for names of sections and rules
#   * RULES_KEYS_PATH - Output path for rendered ${RULES_KEYS_TEMPLATE_PATH} template
#   * RULES_NCO_TEMPLATE_PATH - Template for generated .cpp file that loads and exports rules
#   * RULES_NCO_PATH - Output path for rendered ${RULES_NCO_TEMPLATE_PATH} template
#
# This script reads all JSON files in ${RULES_PATH}, one JSON file per rules.ini section. The rules in each file
# are used to generate const strings, load from INI code and export to INI code using common/rulesections.h types.
#
# Expected structure of JSON files is defined in the schema file 'nco-rules-json-schema.json', use this in your files
# to validate them in VSCode etc.:
#
#   ```json
#   {
#     "$schema": "../../cmake/nco-rules-json-schema.json",
#     "section": "My Section"
#   }
#   ```
#
# Generates four separate blocks of C++ code and provides them as variables to the above templates:
#
#   * RULE_JSON_SOURCES_COMMENTS - Multiline comment lines containing a list of JSON files used to generated rules
#   * RULE_KEYS_DEFINES - Define macros which provide const strings for section and rule names
#   * RULE_PROCESS_CODE - Calls the index operator on a Sections variable/class member for each section
#                         and invokes 'With<IniRuleContext>' with a 'Load' call inside the lambda for each rule
#                         (Sections expected to be of type RuleSections, see: common/rulesections.h)
#
# All JSON files are watched for changes, automatically regenerating code before build and on configure. Adding/deleting
# JSON files is also detected automatically. The template files passed in as params are also watched.
#
##
macro(CHECK_REQUIRED_VARIABLE var_name)
  if(NOT DEFINED ${var_name})
    message(FATAL_ERROR "[NcoRules] The \"${var_name}\" variable must be defined.")
  endif()
endmacro()

CHECK_REQUIRED_VARIABLE(RULES_PATH)
CHECK_REQUIRED_VARIABLE(RULES_NCO_TEMPLATE_PATH)
CHECK_REQUIRED_VARIABLE(RULES_NCO_PATH)
CHECK_REQUIRED_VARIABLE(RULE_KEYS_TEMPLATE_PATH)
CHECK_REQUIRED_VARIABLE(RULE_KEYS_PATH)

function(ResolveRuleValue _RULE_DEFAULT _RULE_VALUE)
  set(RULE_VALUE "${_RULE_DEFAULT}")

  if(${RULE_TYPE} STREQUAL "bool")
    # convert ON/OFF to C boolean literals
    if(${_RULE_DEFAULT})
      set(RULE_VALUE "true")
    else()
      set(RULE_VALUE "false")
    endif()
  elseif(${RULE_TYPE} STREQUAL "float")
      # ensure value is denoted as a float
      set(RULE_VALUE "${RULE_VALUE}f")
  elseif(${RULE_TYPE} STREQUAL "fixed")
      # ensure value wrapped in a fixed class constructor call
      set(RULE_VALUE "fixed(${RULE_VALUE}f)")
  endif()

  set("${_RULE_VALUE}" "${RULE_VALUE}" PARENT_SCOPE)
endfunction()

function(TransformRuleNameToUpperSnakecase _RULE_NAME _RULE_NAME_SNAKE_CASE)
  string(REGEX REPLACE "([A-Z][a-z]+)" "\\1_" RULE_NAME_SNAKE_CASE ${_RULE_NAME})
  string(REGEX REPLACE "_($)" "\\1" RULE_NAME_SNAKE_CASE ${RULE_NAME_SNAKE_CASE})
  string(REGEX REPLACE "([0-9])([A-Z])" "\\1_\\2" RULE_NAME_SNAKE_CASE ${RULE_NAME_SNAKE_CASE})
  string(TOUPPER ${RULE_NAME_SNAKE_CASE} RULE_NAME_SNAKE_CASE)

  set("${_RULE_NAME_SNAKE_CASE}" ${RULE_NAME_SNAKE_CASE} PARENT_SCOPE)
endfunction()

function(LoadRuleProperties _RULES_JSON _RULE_INDEX _RULE_NAME _RULE_TYPE _RULE_DEFAULT)
  string(JSON RULE_OBJECT_JSON GET "${_RULES_JSON}" rules "${_RULE_INDEX}")

  string(JSON RULE_NAME GET "${RULE_OBJECT_JSON}" name)
  string(JSON RULE_TYPE GET "${RULE_OBJECT_JSON}" type)
  string(JSON RULE_DEFAULT GET "${RULE_OBJECT_JSON}" default)

  string(JSON IS_IMPLEMENTED ERROR_VARIABLE JSON_ERROR GET "${RULE_OBJECT_JSON}" implemented)

  if(NOT ${IS_IMPLEMENTED} STREQUAL "OFF")
    # default implemented flag to true
    set(IS_IMPLEMENTED "ON")
  endif()

  set(RULE_NAME ${RULE_NAME} PARENT_SCOPE)
  set(RULE_TYPE ${RULE_TYPE} PARENT_SCOPE)
  set(RULE_DEFAULT ${RULE_DEFAULT} PARENT_SCOPE)
  set(IS_IMPLEMENTED ${IS_IMPLEMENTED} PARENT_SCOPE)
endfunction()

function (ExtractSectionIniCommentFromJson _RULES_JSON _INI_COMMENT)
  string(JSON INI_COMMENT GET "${_RULES_JSON}" ini_comment)

  set("${_INI_COMMENT}" ${INI_COMMENT} PARENT_SCOPE)
endfunction()

function (ExtractSectionNameFromJson _RULES_JSON _SECTION_NAME _SECTION_NAME_UPPER)
  string(JSON SECTION_NAME GET "${_RULES_JSON}" section)

  message(STATUS "[NcoRules] Rule section: ${SECTION_NAME}")

  string(TOUPPER ${SECTION_NAME} SECTION_NAME_UPPER)

  set("${_SECTION_NAME}" ${SECTION_NAME} PARENT_SCOPE)
  set("${_SECTION_NAME_UPPER}" ${SECTION_NAME_UPPER} PARENT_SCOPE)
endfunction()


function(ParseRuleFilePath _RULE_FILE _RELATIVE_RULE_FILE)
  file(RELATIVE_PATH RELATIVE_RULE_FILE "${RULES_PATH}" "${_RULE_FILE}")

  message(STATUS "[NcoRules] Processing rules file: ${RELATIVE_RULE_FILE}")

  set("${_RELATIVE_RULE_FILE}" ${RELATIVE_RULE_FILE} PARENT_SCOPE)
endfunction()

function(SetupRuleCheckBeforeBuild)
  if(_BUILD_TIME_TD_RULES)
    # This prevents attempting to define this check when this file is called by
    # the custom target we define below (think of it like a recursion check).
    return()
  endif()

  message(STATUS "[NcoRules] Setting up pre-build Rules check")

  # Custom target ensures any rule files made after configure and before build
  # are detected and this cmake file is re-ran to regen code. Current variable
  # context is inherited.
  add_custom_target(
    td_rules
    ALL
    BYPRODUCTS
      ${RULES_NCO_PATH}
      ${RULE_KEYS_PATH}
      ${RULES_STATE_FILE}
    COMMENT "[NcoRules] Checking rule files for changes..."
    COMMAND
      ${CMAKE_COMMAND}
      -DRULES_PATH=${RULES_PATH}
      -DRULES_NCO_TEMPLATE_PATH=${RULES_NCO_TEMPLATE_PATH}
      -DRULES_NCO_PATH=${RULES_NCO_PATH}
      -DRULE_KEYS_TEMPLATE_PATH=${RULE_KEYS_TEMPLATE_PATH}
      -DRULE_KEYS_PATH=${RULE_KEYS_PATH}
      -DRULES_STATE_FILE=${RULES_STATE_FILE}
      -D_BUILD_TIME_TD_RULES=TRUE
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

function(ScanForRuleFiles _RULES_STATE_FILE _RULES_FILES _RULES_HASH _FILES_HAVE_CHANGED)
  message(STATUS "[NcoRules] Scanning rule files...")

  file(GLOB RULES_FILES LIST_DIRECTORIES false "${RULES_PATH}/*.json")

  set(RULES_HASH "")

  foreach(RULE_FILE ${RULES_FILES})
    # add hash of file contents to overall rules hash
    file(SHA256 "${RULE_FILE}" FILE_HASH)  
    string(SHA256 RULES_HASH "${RULES_HASH}${FILE_HASH}")

    WatchFileForChanges("${RULE_FILE}")
  endforeach()

  # ensure templates are included in hash so code is generated if they change
  file(SHA256 "${RULES_NCO_TEMPLATE_PATH}" FILE_HASH)  
  string(SHA256 RULES_HASH "${RULES_HASH}${FILE_HASH}")

  file(SHA256 "${RULE_KEYS_TEMPLATE_PATH}" FILE_HASH)  
  string(SHA256 RULES_HASH "${RULES_HASH}${FILE_HASH}")

  WatchFileForChanges("${RULES_NCO_TEMPLATE_PATH}")
  WatchFileForChanges("${RULE_KEYS_TEMPLATE_PATH}")

  # If a previous hash was calculated, and does not match the
  # hash we just calculated, then flag that files have changed.
  set(FILES_HAVE_CHANGED true)

  if(EXISTS "${_RULES_STATE_FILE}")
    file(READ "${_RULES_STATE_FILE}" OLD_RULES_HASH)
    if(OLD_RULES_HASH STREQUAL "${RULES_HASH}")
        set(FILES_HAVE_CHANGED false)
    endif()
  endif()

  set("${_RULES_FILES}" ${RULES_FILES} PARENT_SCOPE)
  set("${_RULES_HASH}" ${RULES_HASH} PARENT_SCOPE)
  set("${_FILES_HAVE_CHANGED}" ${FILES_HAVE_CHANGED} PARENT_SCOPE)
endfunction()

function(Main)
  if(NOT DEFINED "RULES_STATE_FILE")
    set(RULES_STATE_FILE "${CMAKE_BINARY_DIR}/td-rules-hash")
  endif()

  ScanForRuleFiles("${RULES_STATE_FILE}" RULES_FILES RULES_HASH FILES_HAVE_CHANGED)

  if(NOT FILES_HAVE_CHANGED)
    message(STATUS "[NcoRules] No Rule changes detected")
    SetupRuleCheckBeforeBuild()

    return()
  endif()

  message(STATUS "[NcoRules] Generating Rules code...")

  set(RULE_JSON_SOURCES_COMMENTS, "")
  set(RULE_KEYS_DEFINES "")

  set(RULE_PROCESS_CODE "")

  foreach(RULE_FILE ${RULES_FILES})
    ParseRuleFilePath("${RULE_FILE}" RELATIVE_RULE_FILE)

    # load rule definitions from JSON file
    file(READ ${RULE_FILE} RULES_JSON)

    ExtractSectionNameFromJson("${RULES_JSON}" SECTION_NAME SECTION_NAME_UPPER)
    ExtractSectionIniCommentFromJson("${RULES_JSON}" INI_COMMENT)

    message(STATUS "[NcoRules] Generating code for rule section: [${SECTION_NAME}]")
  
    # rulekeys.h comment header
    string(APPEND RULE_JSON_SOURCES_COMMENTS " *   - rules/${RELATIVE_RULE_FILE}\n")

    # rulekeys.h section defines
    set(SECTION_DEFINE "${SECTION_NAME_UPPER}_SECTION")

    string(REGEX REPLACE "[.]" "_" SECTION_DEFINE "${SECTION_DEFINE}")

    string(APPEND RULE_KEYS_DEFINES "\n\n// [${SECTION_NAME}]\n")
    string(APPEND RULE_KEYS_DEFINES "#define ${SECTION_DEFINE} \"${SECTION_NAME}\"\n")

    # rules-nco.cpp
    string(CONCAT SECTION_LEAD_IN "\n    CNC_LOG_INFO(\"Processing rule section: [{}]\", ${SECTION_DEFINE});\n"
                                 "\n    Assert_Section_Not_Present(${SECTION_DEFINE});\n"
                                 "\n"
                                 "    Sections[${SECTION_DEFINE}]\n"
                                 "        .Set_Ini_Comment(ini, \"${INI_COMMENT}\")\n"
                                 "        .With<IniRuleContext>(ini, [](auto& c) {\n"
                                 "            c")
    string(APPEND RULE_PROCESS_CODE "${SECTION_LEAD_IN}")

    string(JSON RULE_COUNT LENGTH "${RULES_JSON}" rules)

    MATH(EXPR RULE_COUNT "${RULE_COUNT}-1")

    foreach(RULE_INDEX RANGE ${RULE_COUNT})
      if(${RULE_INDEX} GREATER 0)
        # rules-nco.cpp
        string(APPEND RULE_PROCESS_CODE "\n             ")
      endif()

      LoadRuleProperties("${RULES_JSON}" "${RULE_INDEX}" RULE_NAME RULE_TYPE RULE_DEFAULT)

      TransformRuleNameToUpperSnakecase("${RULE_NAME}" RULE_NAME_SNAKE_CASE)
      set(RULE_DEFINE "${RULE_NAME_SNAKE_CASE}_RULE")

      # rules-nco.cpp
      ResolveRuleValue("${RULE_DEFAULT}" RULE_VALUE)

      string(APPEND RULE_PROCESS_CODE ".Load(${RULE_DEFINE}).With_Default(${RULE_VALUE})")

      if(${RULE_INDEX} EQUAL ${RULE_COUNT})
        # close call chain for section
        string(APPEND RULE_PROCESS_CODE ";")
      endif()

      if(${IS_IMPLEMENTED} STREQUAL "OFF")
        # add TODO if not implemented yet
        string(APPEND RULE_PROCESS_CODE " // TODO: implement")
      endif()

      if(${RULE_INDEX} EQUAL ${RULE_COUNT})
        # close section lambda parameter and With method call
        string(APPEND RULE_PROCESS_CODE "\n    });")
      endif()

      # rulekeys.h rule defines
      message(STATUS "[NcoRules] Generating code for rule: [${SECTION_NAME}] => ${RULE_NAME}")
      string(APPEND RULE_KEYS_DEFINES "\n#define ${RULE_DEFINE} \"${RULE_NAME}\"")
    endforeach()
  endforeach()

  # render templates
  configure_file(${RULES_NCO_TEMPLATE_PATH} ${RULES_NCO_PATH} @ONLY)
  configure_file(${RULE_KEYS_TEMPLATE_PATH} ${RULE_KEYS_PATH} @ONLY)

  # save rules hash to file, used in ScanForRuleFiles() to detect if changes happen
  message(STATUS "[NcoRules] Saving Rules state")
  file(WRITE "${RULES_STATE_FILE}" "${RULES_HASH}")

  SetupRuleCheckBeforeBuild()
endfunction()

Main()
