# Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

option(SERVERS          "Build worldserver and bnetserver"                            1)
option(SCRIPTS          "Build core with scripts included"                            1)

set(SCRIPT_OPTIONS static dynamic disabled)
file(GLOB SCRIPT_PROJECTS RELATIVE "${CMAKE_SOURCE_DIR}/src/server/scripts/" "${CMAKE_SOURCE_DIR}/src/server/scripts/*")

foreach(SCRIPT_MODULE_PATH ${SCRIPT_PROJECTS})
  if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/src/server/scripts/${SCRIPT_MODULE_PATH}")
    string(TOUPPER "${SCRIPT_MODULE_PATH}" SCRIPT_MODULE_NAME)
    set(SCRIPT_MODULE_NAME "SCRIPTS_${SCRIPT_MODULE_NAME}")

    list(APPEND SCRIPT_MODULES ${SCRIPT_MODULE_NAME})

    set("${SCRIPT_MODULE_NAME}" "static" CACHE STRING "scripts")
    set_property(CACHE "${SCRIPT_MODULE_NAME}" PROPERTY STRINGS ${SCRIPT_OPTIONS})
  endif()
endforeach()

option(TOOLS            "Build map/vmap/mmap extraction/assembler tools"              0)
option(USE_SCRIPTPCH    "Use precompiled headers when compiling scripts"              1)
option(USE_COREPCH      "Use precompiled headers when compiling servers"              1)
# option(DYNAMIC_LINKING  "Enable dynamic library linking.      "                       0)
option(WITH_WARNINGS    "Show all warnings during compile"                            0)
option(WITH_COREDEBUG   "Include additional debug-code in core"                       0)
set(WITH_SOURCE_TREE "no" CACHE STRING "Build the source tree for IDE's.")
set_property(CACHE WITH_SOURCE_TREE PROPERTY STRINGS no flat hierarchical)
option(WITHOUT_GIT      "Disable the GIT testing routines"                            0)
