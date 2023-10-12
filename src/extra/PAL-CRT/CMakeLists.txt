# kindly provided by scivision

cmake_minimum_required(VERSION 3.14...3.25)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(PAL-CRT LANGUAGES C)

option(live "live video using PL3D-KC")

include(ExternalProject)
include(GNUInstallDirs)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}" CACHE PATH "default install path" FORCE)
endif()

# --- PL3D-KC
if(live)
if(UNIX)
  find_package(X11 REQUIRED)
endif()

set(fw_url https://github.com/LMP88959/PL3D-KC.git)
set(fw_tag 3fa35ad)
set(fw_args
-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
-DCMAKE_BUILD_TYPE=Release
-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

set(FW_LIBRARY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}fw${CMAKE_STATIC_LIBRARY_SUFFIX})
set(FW_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})


ExternalProject_Add(fw
GIT_REPOSITORY ${fw_url}
GIT_TAG ${fw_tag}
GIT_SHALLOW true
CMAKE_ARGS ${fw_args}
BUILD_BYPRODUCTS ${FW_LIBRARY}
TLS_VERIFY true
INACTIVITY_TIMEOUT 60
CONFIGURE_HANDLED_BY_BUILD ON
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)
add_library(fw::fw INTERFACE IMPORTED)
target_link_libraries(fw::fw INTERFACE ${FW_LIBRARY}
$<$<BOOL:${UNIX}>:X11::Xext>
$<$<BOOL:${WIN32}>:winmm>
)
target_include_directories(fw::fw INTERFACE ${FW_INCLUDE_DIR})

add_dependencies(fw::fw fw)
endif()

# --- PAL program
add_executable(pal pal_core.c pal.c pal_nes.c pal_main.c ppm_rw.c bmp_rw.c)
target_include_directories(pal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(pal PRIVATE
CMD_LINE_VERSION=$<NOT:$<BOOL:${live}>>
$<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>
)
target_link_libraries(pal PRIVATE
$<$<BOOL:${live}>:fw::fw>
$<$<BOOL:${WIN32}>:winmm>
)

# --- auto-ignore build directory
if(NOT EXISTS ${PROJECT_BINARY_DIR}/.gitignore)
  file(WRITE ${PROJECT_BINARY_DIR}/.gitignore "*")
endif()
