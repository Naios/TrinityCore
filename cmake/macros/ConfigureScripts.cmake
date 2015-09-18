# Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if (DYNAMIC_LINKING)
  set(WITH_DYNAMIC_LINKING ON CACHE INTERNAL "" FORCE)
else()
  unset(WITH_DYNAMIC_LINKING CACHE)

  foreach(SCRIPT_MODULE_NAME ${SCRIPT_MODULES})
    if ("${${SCRIPT_MODULE_NAME}}" STREQUAL "dynamic")
      set(DYNAMIC_LINKING_FORCED ON)
      set(WITH_DYNAMIC_LINKING ON CACHE INTERNAL "" FORCE)
      break()
    endif()
  endforeach()
endif()
