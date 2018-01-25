if (MSVC)
  # Specify the maximum PreCompiled Header memory allocation limit
  # Fixes a compiler-problem when using PCH - the /Ym flag is adjusted by the compiler in MSVC2012,
  # hence we need to set an upper limit with /Zm to avoid discrepancies)
  # (And yes, this is a verified, unresolved bug with MSVC... *sigh*)
  #
  # Note: This workaround was verified to be required on MSVC 2017 as well
  set(COTIRE_PCH_MEMORY_SCALING_FACTOR 500)
endif()

include(cotire)

macro(ADD_CXX_PCH TARGET_NAME_LIST PCH_HEADER)
  # Use the header for every target
  foreach(TARGET_NAME ${TARGET_NAME_LIST})
    # Disable unity builds
    set_target_properties(${TARGET_NAME} PROPERTIES COTIRE_ADD_UNITY_BUILD OFF)

    # Set the prefix header
    set_target_properties(${TARGET_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT ${PCH_HEADER})

    # Workaround for cotire bug: https://github.com/sakra/cotire/issues/138
    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 14)

    # This is a workaround for #21104 (it may be removed in the future as
    # soon as cotire provides a fix for this).
    # We add a dummy file as first source file in the built project where the
    # corresponding PCH files get added to. This is equal to the way we've
    # done this in the previous PCH implementation.
    #
    # Create the filename for the PCH source ("CommonPCH.h" -> "CommonPCH.cpp")
    get_filename_component(PCH_SOURCE ${PCH_HEADER} NAME_WE)
    set(PCH_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/gen_pch/${TARGET_NAME}/${PCH_SOURCE}.cpp")
    #
    # Just touch the file
    if(NOT EXISTS "${PCH_SOURCE}")
      file(WRITE ${PCH_SOURCE} "")
    endif()
    #
    # Add the corresponding PCH source file as the first
    get_target_property(ALL_TARGET_SOURCES ${TARGET_NAME} SOURCES)
    # Insert the PCH source file in front (important),
    # since this is how cotire selects the source file to attach flags to
    list(INSERT ALL_TARGET_SOURCES 0 ${PCH_SOURCE})

    # Re-set the project sources
    set_target_properties(${TARGET_NAME}
                          PROPERTIES SOURCES "${ALL_TARGET_SOURCES}")
  endforeach()

  cotire(${TARGET_NAME_LIST})
endmacro(ADD_CXX_PCH)
