cmake_minimum_required(VERSION 3.15)

project(grey VERSION 1.0 DESCRIPTION "grey")

set(TARGET grey)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(../common.cmake)

# Set version script
set(VERSION_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/grey.map.txt)
if(EXISTS ${VERSION_SCRIPT})
  set(VERSION_SCRIPT_FLAGS "-Wl,--version-script,${VERSION_SCRIPT}" "-Wl,--no-undefined-version")
else()
  message(WARNING "Version script not found at ${VERSION_SCRIPT}")
  set(VERSION_SCRIPT_FLAGS "")
endif()

# Include source and header files
include(sources.cmake)
include(headers.cmake)

# Create INTERFACE target for shared flags
add_library(grey_c_flags INTERFACE)
target_compile_features(grey_c_flags INTERFACE c_std_23)
target_compile_options(
  grey_c_flags
  INTERFACE
  "$<BUILD_INTERFACE:${C_WARNINGS}>"
)
target_link_options(
  grey_c_flags
  INTERFACE
  ${VERSION_SCRIPT_FLAGS}
)

include(ExternalProject)

list(APPEND CMAKE_PREFIX_PATH
  "${CMAKE_BINARY_DIR}/memalloc/lib/cmake/memalloc"
  "${CMAKE_BINARY_DIR}/zot/lib/cmake/zot")

find_package(memalloc QUIET)
if(NOT memalloc_FOUND)
  ExternalProject_Add(
    MemAlloc
    SOURCE_DIR /sdcard/Jay/Projects/xab_laud_ubject/memalloc/
    INSTALL_DIR ${CMAKE_BINARY_DIR}/memalloc
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXEC=OFF
    BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
  )
endif()

# Use relative path for Zot source directory
set(ZOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../xab_laud_ubject/zot)
set(ZOT_INSTALL_DIR ${CMAKE_BINARY_DIR}/zot)

find_package(zot QUIET)
if(NOT zot_FOUND)
  ExternalProject_Add(
    Zot
    SOURCE_DIR /sdcard/Jay/Projects/xab_laud_ubject/zot/
    INSTALL_DIR ${CMAKE_BINARY_DIR}/zot
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
  )
endif()

# Main library setup
add_library(${TARGET} SHARED)
target_sources(${TARGET} PRIVATE ${SOURCE_FILES})
target_link_options(${TARGET} PRIVATE ${VERSION_SCRIPT_FLAGS})
target_link_libraries(${TARGET} PRIVATE grey_c_flags memalloc zot)
target_include_directories(${TARGET} PRIVATE ${ZOT_INSTALL_DIR}/include .)
target_link_directories(${TARGET} PRIVATE ${ZOT_INSTALL_DIR}/lib)
set_target_properties(${TARGET} PROPERTIES LINK_DEPENDS ${VERSION_SCRIPT})
if(NOT memalloc_FOUND)
  target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}/memalloc/include)
  target_link_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}/memalloc/lib)
  add_dependencies(Zot MemAlloc)
endif()
if(NOT zot_FOUND)
  target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}/zot/include)
  target_link_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}/zot/lib)
  add_dependencies(${TARGET} Zot)
endif()

# Option to enable or disable phony main
option(BUILD_PHONY_MAIN "Build phony main" ON)

if(BUILD_PHONY_MAIN)
  # Generate phony main at configure time
  file(WRITE ${CMAKE_BINARY_DIR}/grey_phony_main.c "int game_main();\nint main() { game_main(); return 0; }\n")
  add_executable(grey_phony ${CMAKE_BINARY_DIR}/grey_phony_main.c)
  target_link_libraries(grey_phony PRIVATE ${TARGET})
endif()

# Install targets with preserved header structure
install(TARGETS ${TARGET} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION include)
