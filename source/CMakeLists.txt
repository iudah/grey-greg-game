cmake_minimum_required(VERSION 3.15)

project(
  grey
  VERSION 1.0.1
  DESCRIPTION "grey")

set(TARGET grey)

# Set C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(../common.cmake)

# Option to enable or disable phony main
option(BUILD_PHONY_MAIN "Build phony main" ON)
option(ENABLE_SANITIZERS "Enable the sanitizers" ON)
option(BUILD_RAYLIB "Build Raylib with Grey" ON)

message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

include(GNUInstallDirs)
include(ExternalProject)

set(COMMON_TOOLCHAIN_ARGS "")
if(CMAKE_TOOLCHAIN_FILE)
  list(APPEND COMMON_TOOLCHAIN_ARGS
    "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()
if("${PLATFORM}" STREQUAL "Android")
  set(PLATFORM_IS_ANDROID TRUE)
  set(ENABLE_SANITIZERS OFF)
else()
  set(PLATFORM_IS_ANDROID FALSE)
  if(ANDROID)
    set(BUILD_RAYLIB FALSE)
  endif()
endif()
if(ANDROID AND ${PLATFORM_IS_ANDROID})
  if(NOT ANDROID_NDK)
    set(ANDROID_NDK $ENV{ANDROID_NDK})
  endif()
  list(APPEND COMMON_TOOLCHAIN_ARGS "-DANDROID_ABI=${ANDROID_ABI}"
    "-DANDROID_PLATFORM=${ANDROID_PLATFORM}" "-DANDROID_NDK=${ANDROID_NDK}"
    "-DENABLE_SANITIZERS=${ENABLE_SANITIZERS}")
endif()

find_package(memalloc QUIET)
add_library(memalloc_imported STATIC IMPORTED)
if(NOT TARGET memalloc::memalloc)
  message(STATUS "memalloc not found, adding as ExternalProject")
  ExternalProject_Add(
    MemAlloc
    SOURCE_DIR c:/Jay/memalloc
    # GIT_REPOSITORY "https://github.com/iudah/memalloc.git"
    GIT_TAG origin/master
    GIT_SHALLOW TRUE
    INSTALL_DIR ${CMAKE_BINARY_DIR}/memalloc
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXECUTABLE=OFF
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${COMMON_TOOLCHAIN_ARGS}
    # BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
    # UPDATE_DISCONNECTED TRUE
    BUILD_BYPRODUCTS
    "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}memalloc${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/memalloc/include")

  add_dependencies(memalloc_imported MemAlloc)
  add_library(memalloc::memalloc ALIAS memalloc_imported)
endif()
set_target_properties(
  memalloc_imported
  PROPERTIES
  IMPORTED_LOCATION
  "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}memalloc${CMAKE_STATIC_LIBRARY_SUFFIX}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/memalloc/include")

find_package(zot QUIET)
add_library(zot_imported STATIC IMPORTED)
if(NOT TARGET zot::zot)
  message(STATUS "zot not found, adding as ExternalProject")
  ExternalProject_Add(
    zot
    SOURCE_DIR c:/Jay/zot
    # GIT_REPOSITORY "https://github.com/iudah/zot.git"
    GIT_TAG origin/main
    GIT_SHALLOW TRUE
    INSTALL_DIR ${CMAKE_BINARY_DIR}/zot
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXECUTABLE=OFF
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON ${COMMON_TOOLCHAIN_ARGS}
    # BUILD_ALWAYS OFF
    STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
    # UPDATE_DISCONNECTED TRUE
    BUILD_BYPRODUCTS
    "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zot${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/zot/include")

  add_dependencies(zot_imported zot MemAlloc)
  add_library(zot::zot ALIAS zot_imported)
endif()
set_target_properties(
  zot_imported
  PROPERTIES
  IMPORTED_LOCATION
  "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zot${CMAKE_STATIC_LIBRARY_SUFFIX}"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/zot/include")

if(${BUILD_RAYLIB})
  find_package(raylib QUIET)
  add_library(raylib_imported STATIC IMPORTED)
  if(NOT TARGET raylib::raylib)
    message(STATUS "raylib not found, adding as ExternalProject")

    set(RAYLIB_EXTRA_ARGS "")
    if(${PLATFORM_IS_ANDROID} AND ANDROID)
      set(PLATFORM Android)
      set(RAYLIB_EXTRA_ARGS -DCMAKE_BUILD_TYPE=MinSizeRel -DPLATFORM=Android
        -DCUSTOMIZE_BUILD=ON ${COMMON_TOOLCHAIN_ARGS})
    else()
      if(NOT "${PLATFORM}" STREQUAL "Desktop" AND NOT "${PLATFORM}" STREQUAL
        "Web")
        set(PLATFORM Desktop)
      endif()
      set(RAYLIB_EXTRA_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DPLATFORM=${PLATFORM})
    endif()

    ExternalProject_Add(
      raylib
      SOURCE_DIR c:/Jay/raylib
      # GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
      GIT_SHALLOW TRUE
      GIT_CLONE_DEPTH 1
      INSTALL_DIR ${CMAKE_BINARY_DIR}/raylib
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_EXAMPLES=OFF
      -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      ${RAYLIB_EXTRA_ARGS}
      # BUILD_ALWAYS OFF
      STAMP_DIR ${CMAKE_BINARY_DIR}/stamps
      # UPDATE_DISCONNECTED TRUE
      BUILD_BYPRODUCTS
      "${CMAKE_BINARY_DIR}/raylib/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}raylib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    )

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/raylib/include")

    add_dependencies(raylib_imported raylib)
    add_library(raylib::raylib ALIAS raylib_imported)
  endif()
  set_target_properties(
    raylib_imported
    PROPERTIES
    IMPORTED_LOCATION
    "${CMAKE_BINARY_DIR}/raylib/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}raylib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_BINARY_DIR}/raylib/include")
endif()

if(${PLATFORM_IS_ANDROID})
  set(NATIVE_APP_GLUE ${ANDROID_NDK}/sources/android/native_app_glue/)

  add_library(
    native_app_glue STATIC ${NATIVE_APP_GLUE}/android_native_app_glue.c
    ${NATIVE_APP_GLUE}/android_native_app_glue.h)
  add_library(grey::native_app_glue ALIAS native_app_glue)
  target_include_directories(
    native_app_glue
    PUBLIC $<BUILD_INTERFACE:${NATIVE_APP_GLUE}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

  target_compile_options(native_app_glue PRIVATE -fPIC -Wall
    -Werror=format-security)
endif()

find_package(Threads REQUIRED)

include(sources.cmake)
include(headers.cmake)

if(${PLATFORM_IS_ANDROID})
  add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
  target_compile_options(${TARGET} PRIVATE -fPIC)
  target_compile_definitions(${TARGET} PRIVATE USE_ANDROID_LOG
    TAG="${PROJECT_NAME}")
else()
  add_library(${TARGET} SHARED ${SOURCE_FILES} ${HEADER_FILES})
endif()
add_library(grey::${TARGET} ALIAS ${TARGET})

target_include_directories(
  ${TARGET}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_application>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic/components>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic/entities>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic/systems>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/game_logic/grid>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/graphics>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/physics>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../3rd-party/neon_2_sse>
  $<INSTALL_INTERFACE:include>)

target_link_libraries(${TARGET} PUBLIC m)
target_link_libraries(${TARGET} PUBLIC Threads::Threads)
target_link_libraries(${TARGET} PUBLIC zot::zot)
target_link_libraries(${TARGET} PUBLIC memalloc::memalloc)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows"
  OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN"
  OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
  target_link_libraries(${TARGET} PUBLIC ws2_32 gdi32 winmm)
endif()

if(${PLATFORM_IS_ANDROID})
  target_link_libraries(
    ${TARGET}
    PUBLIC native_app_glue
    log
    android
    EGL
    GLESv2
    OpenSLES
    atomic
    c
    dl)

  target_link_options(
    ${TARGET}
    PUBLIC
    # cmake-format: off
    -u ANativeActivity_onCreate
    # cmake-format: on
    -Wl,--build-id
    -Wl,--no-undefined
    -Wl,-z,noexecstack
    -Wl,-z,relro
    -Wl,-z,now)
endif()

if(${BUILD_RAYLIB})
  target_link_libraries(${TARGET} PUBLIC raylib::raylib)
else()
  target_compile_options(${TARGET} PRIVATE -DNO_RAYLIB=1)
endif()

target_compile_features(${TARGET} PUBLIC c_std_23)

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
      target_link_libraries(${TARGET} PRIVATE -fsanitize=address)
    endif()
    check_c_compiler_flag("-fsanitize=undefined" HAS_UBSAN)
    if(HAS_UBSAN)
      target_compile_options(${TARGET} PRIVATE -fsanitize=undefined)
      target_link_libraries(${TARGET} PRIVATE -fsanitize=undefined)
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

if(BUILD_PHONY_MAIN AND NOT ${PLATFORM_IS_ANDROID})
  file(WRITE ${CMAKE_BINARY_DIR}/grey_phony_main.c
    "int game_main();\nint main() { game_main(); return 0; }\n")
  add_executable(grey_phony ${CMAKE_BINARY_DIR}/grey_phony_main.c)
  target_link_libraries(grey_phony PRIVATE ${TARGET})
endif()

install(
  TARGETS ${TARGET}
  EXPORT GreyTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(${PLATFORM_IS_ANDROID})
  install(
    TARGETS native_app_glue
    EXPORT GreyTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(
  FILES
  "${CMAKE_BINARY_DIR}/zot/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}zot${CMAKE_STATIC_LIBRARY_SUFFIX}"
  "${CMAKE_BINARY_DIR}/memalloc/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}memalloc${CMAKE_STATIC_LIBRARY_SUFFIX}"
  DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY "${CMAKE_BINARY_DIR}/zot/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY "${CMAKE_BINARY_DIR}/memalloc/include/"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(${BUILD_RAYLIB})
  install(
    FILES
    "${CMAKE_BINARY_DIR}/raylib/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}raylib${CMAKE_STATIC_LIBRARY_SUFFIX}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY "${CMAKE_BINARY_DIR}/raylib/include/"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT GreyTargets
  FILE grey-config.cmake
  NAMESPACE grey::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/grey)

export(EXPORT GreyTargets FILE "${CMAKE_BINARY_DIR}/grey-config.cmake")
