cmake_minimum_required(VERSION 3.15)

project(grey VERSION 1.0.0 DESCRIPTION "grey")

set(TARGET grey)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(../common.cmake)

# Option to enable or disable phony main
option(BUILD_PHONY_MAIN "Build phony main" ON)
option(ENABLE_SANITIZERS "Enable the sanitizers" ON)


message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

include(GNUInstallDirs)
include(ExternalProject)

find_package(memalloc QUIET)
add_library(memalloc_imported STATIC IMPORTED)
if(NOT TARGET memalloc::memalloc)
  message(STATUS "memalloc not found, adding as ExternalProject")
  ExternalProject_Add(
    MemAlloc
    GIT_REPOSITORY "https://github.com/iudah/memalloc.git"
    GIT_TAG origin/master
    GIT_SHALLOW TRUE
    INSTALL_DIR ${CMAKE_BINARY_DIR}/memalloc
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXECUTABLE=OFF 
    BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
    UPDATE_DISCONNECTED TRUE
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}memalloc${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/memalloc/include")

  add_dependencies(memalloc_imported MemAlloc)
  add_library(memalloc::memalloc ALIAS memalloc_imported)
endif()
set_target_properties(memalloc_imported PROPERTIES
  IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}memalloc${CMAKE_STATIC_LIBRARY_SUFFIX}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/memalloc/include"
)

find_package(zot QUIET)
add_library(zot_imported STATIC IMPORTED)
if(NOT TARGET zot::zot)
  message(STATUS "zot not found, adding as ExternalProject")
  ExternalProject_Add(
    zot
    GIT_REPOSITORY "https://github.com/iudah/zot.git"
    GIT_TAG origin/main
    GIT_SHALLOW TRUE
    INSTALL_DIR ${CMAKE_BINARY_DIR}/zot
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXECUTABLE=OFF 
    BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
    UPDATE_DISCONNECTED TRUE
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zot${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/zot/include")

  add_dependencies(zot_imported zot MemAlloc)
  add_library(zot::zot ALIAS zot_imported)
endif()
set_target_properties(zot_imported PROPERTIES
  IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zot${CMAKE_STATIC_LIBRARY_SUFFIX}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/zot/include"
)

include(sources.cmake)
include(headers.cmake)

add_library(${TARGET} SHARED ${SOURCE_FILES} ${HEADER_FILES})
add_library(grey::${TARGET} ALIAS ${TARGET})

target_include_directories(${TARGET}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_application>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic/components>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd-party/neon_2_sse>
  $<INSTALL_INTERFACE:include>
)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
  target_link_libraries(${TARGET} PUBLIC ws2_32)
endif()
target_link_libraries(${TARGET} PUBLIC m)
target_link_libraries(${TARGET} PUBLIC pthread)
target_link_libraries(${TARGET} PUBLIC zot::zot)
target_link_libraries(${TARGET} PUBLIC memalloc::memalloc)
target_compile_features(${TARGET} PUBLIC c_std_23)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /Zi /Od /RTC1)
    target_link_options(${TARGET} PRIVATE /DEBUG)
  else()
    target_compile_options(${TARGET} PRIVATE -g -O0 -fno-omit-frame-pointer -DZ_USE_ASAN=1)
  endif()
  if(ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(${TARGET} PRIVATE -fsanitize=address -fsanitize=undefined)
  endif()
endif()

if(NOT MSVC)
  include(CheckCCompilerFlag)
  if((CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64"))
    check_c_compiler_flag("-mfpu=neon" HAS_NEON_FLAG)
    if(HAS_NEON_FLAG)
      target_compile_options(${TARGET} PRIVATE -mfpu=neon)
    endif()
  else()
    check_c_compiler_flag("-msse3" HAS_MSSE3)
    if(HAS_MSSE3)
      target_compile_options(${TARGET} PRIVATE -msse3 -march=native)
    endif()
  endif()
endif()

if(BUILD_PHONY_MAIN)
  # Generate phony main at configure time
  file(WRITE ${CMAKE_BINARY_DIR}/grey_phony_main.c "int game_main();\nint main() { game_main(); return 0; }\n")
  add_executable(grey_phony ${CMAKE_BINARY_DIR}/grey_phony_main.c)
  target_link_libraries(grey_phony PRIVATE ${TARGET})
endif()


install(TARGETS ${TARGET}
  EXPORT GreyTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT GreyTargets
  FILE grey-config.cmake
  NAMESPACE grey::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grey)

export(EXPORT GreyTargets
  FILE "${CMAKE_BINARY_DIR}/grey-config.cmake")
