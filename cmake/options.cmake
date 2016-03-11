# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

option(SERVERS          "Build worldserver and bnetserver"                            1)

set(SCRIPT_OPTIONS static dynamic disabled)
file(GLOB SCRIPT_PROJECTS RELATIVE "${CMAKE_SOURCE_DIR}/src/server/scripts/" "${CMAKE_SOURCE_DIR}/src/server/scripts/*")

# Set the default option value according to the script projects /CMakeLists.txt
foreach(SCRIPT_MODULE_PATH ${SCRIPT_PROJECTS})
  if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/server/scripts/${SCRIPT_MODULE_PATH}")

    # Save the script module names for later usage
    string(TOUPPER "${SCRIPT_MODULE_PATH}" SCRIPT_MODULE_NAME)
    set(SCRIPT_MODULE_NAME "SCRIPTS_${SCRIPT_MODULE_NAME}")
    list(APPEND SCRIPT_MODULES ${SCRIPT_MODULE_NAME})

    # Include the variables from the CMakeLists.txt inside the script project
    include("${CMAKE_SOURCE_DIR}/src/server/scripts/${SCRIPT_MODULE_PATH}/CMakeLists.txt" OPTIONAL RESULT_VARIABLE DEFAULTS_FILE_FOUND)

    # When no CMakeLists.txt inside the script project was found use default values
    if (NOT DEFAULTS_FILE_FOUND)
      # default linkage of module for SCRIPTS=0 (basic scripts only)
      set(PREFER_BASIC_SCRIPTS   "disabled")
      # default linkage of module for SCRIPTS=1 (prefer static linking)
      set(PREFER_STATIC_SCRIPTS  "static")
      # default linkage of module for SCRIPTS=2 (prefer dynamic linking)
      set(PREFER_DYNAMIC_SCRIPTS "dynamic")
    endif()

    # Set the default value according to the SCRIPTS variable
    if (NOT SCRIPTS)
      # Default value if SCRIPTS isn't defined
      set(DEFAULT_VALUE ${PREFER_STATIC_SCRIPTS})
    elseif(${SCRIPTS} EQUAL 0)
      set(DEFAULT_VALUE ${PREFER_BASIC_SCRIPTS})
    elseif(${SCRIPTS} EQUAL 1)
      set(DEFAULT_VALUE ${PREFER_STATIC_SCRIPTS})
    elseif(${SCRIPTS} EQUAL 2)
      set(DEFAULT_VALUE ${PREFER_DYNAMIC_SCRIPTS})
    else()
      message(FATAL_ERROR "Invalid SCRIPTS=${SCRIPTS} value!")
    endif()

    set("${SCRIPT_MODULE_NAME}" ${DEFAULT_VALUE} CACHE STRING "scripts")
    set_property(CACHE "${SCRIPT_MODULE_NAME}" PROPERTY STRINGS ${SCRIPT_OPTIONS})
  endif()
endforeach()

unset(PREFER_BASIC_SCRIPTS)
unset(PREFER_STATIC_SCRIPTS)
unset(PREFER_DYNAMIC_SCRIPTS)

option(TOOLS            "Build map/vmap/mmap extraction/assembler tools"              0)
option(USE_SCRIPTPCH    "Use precompiled headers when compiling scripts"              1)
option(USE_COREPCH      "Use precompiled headers when compiling servers"              1)
option(DYNAMIC_LINKING  "Enable dynamic library linking (experimental)."              0)
option(WITH_WARNINGS    "Show all warnings during compile"                            0)
option(WITH_COREDEBUG   "Include additional debug-code in core"                       0)
set(WITH_SOURCE_TREE "no" CACHE STRING "Build the source tree for IDE's.")
set_property(CACHE WITH_SOURCE_TREE PROPERTY STRINGS no flat hierarchical)
option(WITHOUT_GIT      "Disable the GIT testing routines"                            0)
