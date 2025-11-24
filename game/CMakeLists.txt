cmake_minimum_required(VERSION 3.15)

project(greg VERSION 2.0.0 DESCRIPTION "greg")

set(TARGET greg)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(../common.cmake)

option(ENABLE_SANITIZERS "Enable the sanitizers" OFF)

message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

include(ExternalProject)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/cmake/memalloc"
  "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/cmake/zot")

find_package(grey QUIET)
if(NOT TARGET grey::grey)
  message(STATUS "grey not found")
endif()

# Include source and header files
include(sources.cmake)
include(headers.cmake)

add_executable(${TARGET} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${TARGET} PUBLIC pthread)
target_link_libraries(${TARGET} PUBLIC m)
target_link_libraries(${TARGET} PRIVATE grey::grey)
target_compile_features(${TARGET} PUBLIC c_std_23)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /Zi /Od /RTC1)
    target_link_options(${TARGET} PRIVATE /DEBUG)
  else()
    target_compile_options(${TARGET} PRIVATE -g -O0 -fno-omit-frame-pointer)
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

include(GNUInstallDirs)

install(TARGETS ${TARGET}
  EXPORT GregTargets
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT GregTargets
  FILE greg-config.cmake
  NAMESPACE greg::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/greg)

export(EXPORT GregTargets
  FILE "${CMAKE_BINARY_DIR}/greg-config.cmake")
