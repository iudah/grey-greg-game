cmake_minimum_required(VERSION 3.15)

project(
  greg
  VERSION 2.0.1
  DESCRIPTION "greg")

set(TARGET greg)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(../common.cmake)

option(ENABLE_SANITIZERS "Enable the sanitizers" ON)
option(BUILD_RAYLIB "Build Raylib with Grey" ON)

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

include(ExternalProject)

list(APPEND CMAKE_PREFIX_PATH
     "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/cmake/memalloc"
     "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/cmake/zot")

find_package(Threads REQUIRED)

find_package(grey QUIET)
if(NOT TARGET grey::grey)
  message(STATUS "grey not found")
endif()

# Include source and header files
include(sources.cmake)
include(headers.cmake)

if("${PLATFORM}" STREQUAL "Android")
  set(PLATFORM_IS_ANDROID TRUE)
  set(ENABLE_SANITIZERS OFF)
else()
  set(PLATFORM_IS_ANDROID FALSE)
  if(ANDROID)
    set(BUILD_RAYLIB FALSE)
  endif()
endif()

if(${PLATFORM_IS_ANDROID})
  set(PLATFORM Android)
  add_library(${TARGET} SHARED ${SOURCE_FILES} ${HEADER_FILES})
  target_compile_definitions(${TARGET} PRIVATE TAG="${PROJECT_NAME}"
                                               USE_ANDROID_LOG)
else()
  add_executable(${TARGET} ${SOURCE_FILES} ${HEADER_FILES})
endif()
target_include_directories(
  ${TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include>)
target_link_libraries(${TARGET} PUBLIC Threads::Threads)
target_link_libraries(${TARGET} PUBLIC m)
target_link_libraries(${TARGET} PRIVATE grey::grey)
target_compile_features(${TARGET} PUBLIC c_std_23)

if(NOT ${BUILD_RAYLIB})
  target_compile_definitions(${TARGET} PRIVATE -NO_RAYLIB=1)
endif()

include(CheckCCompilerFlag)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL
                                        "RelWithDebInfo")
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /Zi /Od /RTC1)
    target_link_options(${TARGET} PRIVATE /DEBUG)
  else()
    target_compile_options(${TARGET} PRIVATE -g -O0 -fno-omit-frame-pointer)
  endif()
  if(ENABLE_SANITIZERS AND NOT MSVC)
    check_c_compiler_flag("-fsanitize=address" HAS_ASAN)
    if(HAS_ASAN)
      target_compile_options(${TARGET} PRIVATE -fsanitize=address)
      target_link_options(${TARGET} PRIVATE -fsanitize=address)
    endif()
    check_c_compiler_flag("-fsanitize=undefined" HAS_UBSAN)
    if(HAS_UBSAN)
      target_compile_options(${TARGET} PRIVATE -fsanitize=undefined)
      target_link_options(${TARGET} PRIVATE -fsanitize=undefined)
    endif()
  endif()
endif()

if(NOT MSVC)
  if((CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES
                                              "aarch64"))
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

include(GNUInstallDirs)

install(
  TARGETS ${TARGET}
  EXPORT GregTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT GregTargets
  FILE greg-config.cmake
  NAMESPACE greg::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/greg)

export(EXPORT GregTargets FILE "${CMAKE_BINARY_DIR}/greg-config.cmake")
