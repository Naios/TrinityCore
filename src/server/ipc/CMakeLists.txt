# Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

find_package(ZMQ REQUIRED)

file(GLOB_RECURSE sources_ipc *.cpp *.h)

set(ipc_SRCS
  ${sources_ipc}
)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dep/zmqpp
  ${CMAKE_SOURCE_DIR}/src/common
  ${CMAKE_SOURCE_DIR}/src/server/shared
  ${ZMQ_INCLUDE_DIR}
)

if (WITH_DYNAMIC_LINKING)
  add_definitions(-DTRINITY_API_EXPORT_IPC)

  add_library(ipc SHARED ${ipc_SRCS})

  if( WIN32 AND MSVC)
    set(ADDITIONAL_WINDOWS_LIBRARIES ws2_32)
  endif()

  target_link_libraries(ipc
    zmqpp
    ${ADDITIONAL_WINDOWS_LIBRARIES}
    ${ZMQ_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
  )

  if( UNIX )
    install(TARGETS ipc DESTINATION lib)
  elseif( WIN32 )
    install(TARGETS ipc
      RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()

else()
  add_library(ipc STATIC ${ipc_SRCS})
endif()
