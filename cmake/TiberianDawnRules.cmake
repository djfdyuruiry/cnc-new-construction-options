# Rules code generator, allows keeping game rules in one place
# for a single source of truth.

function(ResolveRuleValue _RULE_DEFAULT _RULE_VALUE)
  set(RULE_VALUE "${_RULE_DEFAULT}")

  if(${RULE_TYPE} STREQUAL "bool")
    # covert ON/OFF to C boolean literals
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

  string(JSON RULE_NAME GET ${RULE_OBJECT_JSON} name)
  string(JSON RULE_TYPE GET ${RULE_OBJECT_JSON} type)
  string(JSON RULE_DEFAULT GET ${RULE_OBJECT_JSON} default)

  string(JSON IS_IMPLEMENTED ERROR_VARIABLE JSON_ERROR GET ${RULE_OBJECT_JSON} implemented)

  if(NOT ${IS_IMPLEMENTED} STREQUAL "OFF")
    # default implemented flag to true
    set(IS_IMPLEMENTED "ON")
  endif()

  set(RULE_NAME ${RULE_NAME} PARENT_SCOPE)
  set(RULE_TYPE ${RULE_TYPE} PARENT_SCOPE)
  set(RULE_DEFAULT ${RULE_DEFAULT} PARENT_SCOPE)
  set(IS_IMPLEMENTED ${IS_IMPLEMENTED} PARENT_SCOPE)
endfunction()

function (ExtractSectionNameFromJson _RULES_JSON _SECTION_NAME _SECTION_NAME_UPPER)
  string(JSON SECTION_NAME GET ${_RULES_JSON} section)

  message(STATUS "Rule section: ${SECTION_NAME}")

  string(TOUPPER ${SECTION_NAME} SECTION_NAME_UPPER)

  set("${_SECTION_NAME}" ${SECTION_NAME} PARENT_SCOPE)
  set("${_SECTION_NAME_UPPER}" ${SECTION_NAME_UPPER} PARENT_SCOPE)
endfunction()

function(ParseRuleFilePath _RULE_FILE _RELATIVE_RULE_FILE)
  file(RELATIVE_PATH RELATIVE_RULE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/rules "${_RULE_FILE}")

  message(STATUS "Processing rules file: ${RELATIVE_RULE_FILE}")

  set("${_RELATIVE_RULE_FILE}" ${RELATIVE_RULE_FILE} PARENT_SCOPE)
endfunction()

function(ScanForRuleFiles _RULES_FILES)
  file(RELATIVE_PATH RELATIVE_SOURCE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  message(STATUS "Processing rules files in ${RELATIVE_SOURCE_DIR}/rules/*.json")

  file(GLOB_RECURSE RULES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/rules/*.json")

  set("${_RULES_FILES}" ${RULES_FILES} PARENT_SCOPE)
endfunction()

function(Main)
  message(STATUS "=== TiberianDawnRules.cmake start ===")

  set(RULE_JSON_SOURCES_COMMENTS, "")
  set(RULE_KEYS_DEFINES "")

  set(RULE_PROCESS_CODE "")
  set(RULE_EXPORT_CODE "")

  # find JSON files
  ScanForRuleFiles(RULES_FILES)

  foreach(RULE_FILE ${RULES_FILES})
    ParseRuleFilePath("${RULE_FILE}" RELATIVE_RULE_FILE)

    # load rule definitions from JSON file
    file(READ ${RULE_FILE} RULES_JSON)

    ExtractSectionNameFromJson("${RULES_JSON}" SECTION_NAME SECTION_NAME_UPPER)

    message(STATUS "Generating code for rule section: [${SECTION_NAME}]")
  
    # rulekeys.h comment header
    string(APPEND RULE_JSON_SOURCES_COMMENTS " *   - rules/${RELATIVE_RULE_FILE}\n")

    # rulekeys.h section defines
    set(SECTION_DEFINE "${SECTION_NAME_UPPER}_SECTION")

    string(REGEX REPLACE "[.]" "_" SECTION_DEFINE "${SECTION_DEFINE}")

    string(APPEND RULE_KEYS_DEFINES "\n\n// [${SECTION_NAME}]\n")
    string(APPEND RULE_KEYS_DEFINES "#define ${SECTION_DEFINE} \"${SECTION_NAME}\"\n")

    # rules-nco.cpp
    string(CONCAT SECTION_LEAD_IN "\n    CNC_LOG_INFO(\"Processing rule section: [{}]\", ${SECTION_DEFINE});\n"
                                 "\n"
                                 "    Sections[${SECTION_DEFINE}].With<IniRuleContext>(ini, [](auto& c) {\n"
                                 "        c")
    string(APPEND RULE_PROCESS_CODE "${SECTION_LEAD_IN}")

    string(CONCAT SECTION_LEAD_IN "\n    CNC_LOG_INFO(\"Exporting rule section: [{}]\", ${SECTION_DEFINE});\n"
                                 "\n"
                                 "    Sections[${SECTION_DEFINE}].With<IniRuleContext>(ini, [](auto& c) {\n"
                                 "        c")
    string(APPEND RULE_EXPORT_CODE "${SECTION_LEAD_IN}")


    string(JSON RULE_COUNT LENGTH ${RULES_JSON} rules)
    MATH(EXPR RULE_COUNT "${RULE_COUNT}-1")

    foreach(RULE_INDEX RANGE ${RULE_COUNT})
      if(${RULE_INDEX} GREATER 0)
        # rules-nco.cpp
        string(APPEND RULE_PROCESS_CODE "\n         ")
        string(APPEND RULE_EXPORT_CODE "\n         ")
      endif()

      LoadRuleProperties("${RULES_JSON}" "${RULE_INDEX}" RULE_NAME RULE_TYPE RULE_DEFAULT)

      TransformRuleNameToUpperSnakecase("${RULE_NAME}" RULE_NAME_SNAKE_CASE)
      set(RULE_DEFINE "${RULE_NAME_SNAKE_CASE}_RULE")

      # rules-nco.cpp
      ResolveRuleValue("${RULE_DEFAULT}" RULE_VALUE)

      string(APPEND RULE_PROCESS_CODE ".Load(${RULE_DEFINE}).With_Default(${RULE_VALUE})")
      string(APPEND RULE_EXPORT_CODE ".template Save<${RULE_TYPE}>(${RULE_DEFINE})")

      if(${RULE_INDEX} EQUAL ${RULE_COUNT})
        # close call chain for section
        string(APPEND RULE_PROCESS_CODE ";")
        string(APPEND RULE_EXPORT_CODE ";")
      endif()

      if(${IS_IMPLEMENTED} STREQUAL "OFF")
        # add TODO if not implemented yet
        string(APPEND RULE_PROCESS_CODE " // TODO: implement")
        string(APPEND RULE_EXPORT_CODE " // TODO: implement")
      endif()

      if(${RULE_INDEX} EQUAL ${RULE_COUNT})
        # close section lambda parameter and With method call
        string(APPEND RULE_PROCESS_CODE "\n    });")
        string(APPEND RULE_EXPORT_CODE "\n    });")
      endif()

      # rulekeys.h rule defines
      message(STATUS "Generating code for rule: [${SECTION_NAME}] => ${RULE_NAME}")
      string(APPEND RULE_KEYS_DEFINES "\n#define ${RULE_DEFINE} \"${RULE_NAME}\"")
    endforeach()
  endforeach()

  # render templates
  set(RULES_NCO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rules-nco.cpp)
  set(RULE_KEYS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rulekeys.h)

  configure_file(${RULES_NCO_PATH}.in ${RULES_NCO_PATH} @ONLY)
  configure_file(${RULE_KEYS_PATH}.in ${RULE_KEYS_PATH} @ONLY)

  message(STATUS "=== TiberianDawnRules.cmake end ===")
endfunction()

Main()
