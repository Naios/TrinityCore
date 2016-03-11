# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Enable precompiled headers when using the GCC compiler.
if (USE_SCRIPTPCH)
  set(scripts_STAT_PCH_HDR ScriptPCH.h)
  set(scripts_STAT_PCH_SRC ScriptPCH.cpp)
endif ()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/cppformat
  ${CMAKE_SOURCE_DIR}/dep/g3dlite/include
  ${CMAKE_SOURCE_DIR}/dep/recastnavigation/Detour/Include
  ${CMAKE_SOURCE_DIR}/src/common/
  ${CMAKE_SOURCE_DIR}/src/common/Collision
  ${CMAKE_SOURCE_DIR}/src/common/Collision/Management
  ${CMAKE_SOURCE_DIR}/src/common/Collision/Maps
  ${CMAKE_SOURCE_DIR}/src/common/Collision/Models
  ${CMAKE_SOURCE_DIR}/src/common/Configuration
  ${CMAKE_SOURCE_DIR}/src/common/Debugging
  ${CMAKE_SOURCE_DIR}/src/common/Logging
  ${CMAKE_SOURCE_DIR}/src/common/Threading
  ${CMAKE_SOURCE_DIR}/src/common/Utilities
  ${CMAKE_SOURCE_DIR}/src/server/database/Database
  ${CMAKE_SOURCE_DIR}/src/server/game/Accounts
  ${CMAKE_SOURCE_DIR}/src/server/game/Achievements
  ${CMAKE_SOURCE_DIR}/src/server/game/Addons
  ${CMAKE_SOURCE_DIR}/src/server/game/AI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/CoreAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/ScriptedAI
  ${CMAKE_SOURCE_DIR}/src/server/game/AI/SmartScripts
  ${CMAKE_SOURCE_DIR}/src/server/game/AuctionHouse
  ${CMAKE_SOURCE_DIR}/src/server/game/AuctionHouseBot
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefield
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlefield/Zones
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds
  ${CMAKE_SOURCE_DIR}/src/server/game/Battlegrounds/Zones
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat
  ${CMAKE_SOURCE_DIR}/src/server/game/Chat/Channels
  ${CMAKE_SOURCE_DIR}/src/server/game/Combat
  ${CMAKE_SOURCE_DIR}/src/server/game/Conditions
  ${CMAKE_SOURCE_DIR}/src/server/game/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/game/DungeonFinding
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/AreaTrigger
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Corpse
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Creature
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/DynamicObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/GameObject
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Item/Container
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Object/Updates
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Pet
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Player
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Transport
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Unit
  ${CMAKE_SOURCE_DIR}/src/server/game/Entities/Vehicle
  ${CMAKE_SOURCE_DIR}/src/server/game/Events
  ${CMAKE_SOURCE_DIR}/src/server/game/Globals
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Cells
  ${CMAKE_SOURCE_DIR}/src/server/game/Grids/Notifiers
  ${CMAKE_SOURCE_DIR}/src/server/game/Groups
  ${CMAKE_SOURCE_DIR}/src/server/game/Guilds
  ${CMAKE_SOURCE_DIR}/src/server/game/Handlers
  ${CMAKE_SOURCE_DIR}/src/server/game/Instances
  ${CMAKE_SOURCE_DIR}/src/server/game/Loot
  ${CMAKE_SOURCE_DIR}/src/server/game/Mails
  ${CMAKE_SOURCE_DIR}/src/server/game/Maps
  ${CMAKE_SOURCE_DIR}/src/server/game/Miscellaneous
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/MovementGenerators
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Spline
  ${CMAKE_SOURCE_DIR}/src/server/game/Movement/Waypoints
  ${CMAKE_SOURCE_DIR}/src/server/game/OutdoorPvP
  ${CMAKE_SOURCE_DIR}/src/server/game/Pools
  ${CMAKE_SOURCE_DIR}/src/server/game/Quests
  ${CMAKE_SOURCE_DIR}/src/server/game/Reputation
  ${CMAKE_SOURCE_DIR}/src/server/game/Scripting
  ${CMAKE_SOURCE_DIR}/src/server/game/Server
  ${CMAKE_SOURCE_DIR}/src/server/game/Server/Packets
  ${CMAKE_SOURCE_DIR}/src/server/game/Server/Protocol
  ${CMAKE_SOURCE_DIR}/src/server/game/Skills
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells
  ${CMAKE_SOURCE_DIR}/src/server/game/Spells/Auras
  ${CMAKE_SOURCE_DIR}/src/server/game/Support
  ${CMAKE_SOURCE_DIR}/src/server/game/Texts
  ${CMAKE_SOURCE_DIR}/src/server/game/Tickets
  ${CMAKE_SOURCE_DIR}/src/server/game/Tools
  ${CMAKE_SOURCE_DIR}/src/server/game/Warden
  ${CMAKE_SOURCE_DIR}/src/server/game/Weather
  ${CMAKE_SOURCE_DIR}/src/server/game/World
  ${CMAKE_SOURCE_DIR}/src/server/ipc
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${CMAKE_SOURCE_DIR}/src/server/shared/DataStores
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic
  ${CMAKE_SOURCE_DIR}/src/server/shared/Dynamic/LinkedReference
  ${CMAKE_SOURCE_DIR}/src/server/shared/Packets
  ${MYSQL_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${VALGRIND_INCLUDE_DIR}
)

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

message("")
message("* Processing scripts...")

foreach(SCRIPT_PROJECT ${SCRIPT_PROJECTS})
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_PROJECT}")

    # Extract the current status of the script project which could marked as:
    # - static
    # - dynamic
    # - disabled
    string(TOUPPER "${SCRIPT_PROJECT}" SCRIPT_PROJECT_VAR_NAME)
    set(SCRIPT_PROJECT_VAR_NAME "SCRIPTS_${SCRIPT_PROJECT_VAR_NAME}")
    set(SCRIPT_PROJECT_STATUS "${${SCRIPT_PROJECT_VAR_NAME}}")

    message("  - ${SCRIPT_PROJECT} scripts -> [${SCRIPT_PROJECT_STATUS}]")

    string(TOLOWER "scripts_${SCRIPT_PROJECT}" SCRIPT_LIB_NAME)
    if(WIN32)
      set(SCRIPT_LIB_NAME "${SCRIPT_LIB_NAME}.dll")
    else()
      set(SCRIPT_LIB_NAME "lib${SCRIPT_LIB_NAME}.so")
    endif()

    if (NOT "${SCRIPT_PROJECT_STATUS}" STREQUAL "disabled")

      # Add all files of the script subdirectory to the project:
      file(GLOB_RECURSE SCRIPT_PROJECT_SOURCES ${SCRIPT_PROJECT}/*.cpp ${SCRIPT_PROJECT}/*.h ${SCRIPT_PROJECT}/*.hpp)

      # Determine the loader function ("Add##${NameOfDirectory}##Scripts()")
      set(SCRIPT_PROJECT_LOADER_FUNCTION_NAME "Add${SCRIPT_PROJECT}Scripts()")

      set(TRINITY_SCRIPTS_FORWARD_DECL
        "void ${SCRIPT_PROJECT_LOADER_FUNCTION_NAME};\n")
      set(TRINITY_SCRIPTS_INVOKE
        "    ${SCRIPT_PROJECT_LOADER_FUNCTION_NAME};\n")

      if ("${SCRIPT_PROJECT_STATUS}" STREQUAL "static")
        # Static link the script module

        # Add all static linked scripts to "script library"
        set(scripts_STAT_SRC
          ${scripts_STAT_SRC}
          ${SCRIPT_PROJECT_SOURCES}
        )

        list(APPEND REGISTERED_STATIC_SCRIPT_MODULES ${SCRIPT_PROJECT})

        set(TRINITY_SCRIPTS_FORWARD_DECL_LIST
          "${TRINITY_SCRIPTS_FORWARD_DECL_LIST}${TRINITY_SCRIPTS_FORWARD_DECL}")
        set(TRINITY_SCRIPTS_INVOKE_LIST
          "${TRINITY_SCRIPTS_INVOKE_LIST}${TRINITY_SCRIPTS_INVOKE}")

        list(APPEND SCRIPTS_TO_UNINSTALL ${SCRIPT_LIB_NAME})

      elseif ("${SCRIPT_PROJECT_STATUS}" STREQUAL "dynamic")

        # Dynamic link the script module
        string(TOLOWER "scripts_${SCRIPT_PROJECT}" SCRIPT_LIB_NAME)

        # Add the script module to the worldserver dependencies
        list(APPEND REGISTERED_DYNAMIC_SCRIPT_MODULES ${SCRIPT_LIB_NAME})

        # Pass parameters to the scriptloader
        set(TRINITY_IS_DYNAMIC_SCRIPTLOADER ON)

        configure_file(
          "${CMAKE_CURRENT_SOURCE_DIR}/ScriptLoader.cpp.in.cmake"
          "${BUILDDIR}/scriptloader/${SCRIPT_PROJECT}/ScriptLoader.cpp"
        )

        unset(TRINITY_IS_DYNAMIC_SCRIPTLOADER)

        add_library(${SCRIPT_LIB_NAME} SHARED
          ${SCRIPT_PROJECT_SOURCES}
          ${scripts_STAT_PCH_SRC}
          "${BUILDDIR}/scriptloader/${SCRIPT_PROJECT}/ScriptLoader.cpp"
        )

        target_link_libraries(${SCRIPT_LIB_NAME}
          game
          shared
          database
          common
          g3dlib
          Detour
          format
          ${JEMALLOC_LIBRARY}
          ${READLINE_LIBRARY}
          ${TERMCAP_LIBRARY}
          ${MYSQL_LIBRARY}
          ${OPENSSL_LIBRARIES}
          ${ZLIB_LIBRARIES}
          ${CMAKE_THREAD_LIBS_INIT}
          ${Boost_LIBRARIES}
        )

        if( UNIX )
          install(TARGETS ${SCRIPT_LIB_NAME}
            DESTINATION "bin/scripts"
            COMPONENT ${SCRIPT_LIB_NAME}
          )
        elseif( WIN32 )
          install(TARGETS ${SCRIPT_LIB_NAME}
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/scripts"
            COMPONENT ${SCRIPT_LIB_NAME}
            LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/scripts"
            COMPONENT ${SCRIPT_LIB_NAME}
          )

          if( MSVC )
            # Place the script modules in the script subdirectory
            set_target_properties(${SCRIPT_LIB_NAME} PROPERTIES
              RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug/scripts"
              RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release/scripts"
              RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/scripts"
              RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin/MinSizeRel/scripts"
            )
          endif()
        endif()

        add_dependencies(${SCRIPT_LIB_NAME}
          revision_data.h
        )
      else()
        message(SEND_ERROR "Invalid option for var ${SCRIPT_PROJECT_VAR_NAME} = ${SCRIPT_PROJECT_STATUS}!")
      endif()

    else()
      # Mark disabled modules for deletion
      list(APPEND SCRIPTS_TO_UNINSTALL ${SCRIPT_LIB_NAME})
    endif()
  endif()
endforeach()

if (REGISTERED_DYNAMIC_SCRIPT_MODULES)
  # Transfer the list of dynamic script moduels to the parent scope
  set(REGISTERED_DYNAMIC_SCRIPT_MODULES ${REGISTERED_DYNAMIC_SCRIPT_MODULES} PARENT_SCOPE)
endif()

set(TRINITY_SCRIPTS_FORWARD_DECL ${TRINITY_SCRIPTS_FORWARD_DECL_LIST})
set(TRINITY_SCRIPTS_INVOKE       ${TRINITY_SCRIPTS_INVOKE_LIST})

# Unset the SCRIPT_PROJECT var to get an empty string.
unset(SCRIPT_PROJECT)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/ScriptLoader.cpp.in.cmake"
  "${BUILDDIR}/scriptloader/ScriptLoader.cpp"
)

set(scripts_STAT_SRC
  ${scripts_STAT_SRC}
  ${BUILDDIR}/scriptloader/ScriptLoader.cpp
)

# Create a project for all static linked scripts.
add_library(scripts STATIC
  ${scripts_STAT_SRC}
  ${scripts_STAT_PCH_SRC}
)

if (USE_SCRIPTPCH)
  set(SCRIPT_PROJECTS_FOR_PCH
    ${REGISTERED_DYNAMIC_SCRIPT_MODULES}
    scripts
  )

  add_cxx_pch("${REGISTERED_DYNAMIC_SCRIPT_MODULES}" ${scripts_STAT_PCH_HDR} ${scripts_STAT_PCH_SRC})
endif()

# Remove all shared libraries in the installl directory which
# are contained in the static library already.
install(CODE "
  set(SCRIPTS_TO_UNINSTALL \"${SCRIPTS_TO_UNINSTALL}\")
  foreach(SCRIPT_TO_UNINSTALL \${SCRIPTS_TO_UNINSTALL})
    set(SCRIPT_TO_UNINSTALL \"\${CMAKE_INSTALL_PREFIX}/scripts/\${SCRIPT_TO_UNINSTALL}\")

    if (EXISTS \"\${SCRIPT_TO_UNINSTALL}\")
      message(STATUS \"Removing \${SCRIPT_TO_UNINSTALL}\")
      file(REMOVE \"\${SCRIPT_TO_UNINSTALL}\")
    endif()
  endforeach()
")

message("")
