# SpAll — lightweight GLSL editor with in-process SPIR-V compile/disassemble.
#
# Layout:
#   core/   pure C++ + QtCore (no Widgets) — compiler, disassembler, reflection
#   ui/     Qt Widgets — editor, highlighters, panels
#   app/    executable entry point
#   tests/  GUI-less smoke test
#
# Third-party dependencies are declared in cmake/Dependencies.cmake.

cmake_minimum_required(VERSION 3.22)

project(SpAll
    VERSION 1.0.0
    DESCRIPTION "Lightweight GLSL editor with in-process SPIR-V compile and disassembly"
    LANGUAGES C CXX)

# --- Language standard -------------------------------------------------------
# Set before any FetchContent_MakeAvailable so third-party projects inherit a
# consistent standard.
set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_EXTENSIONS OFF CACHE BOOL "" FORCE)
set(CMAKE_C_STANDARD 11 CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- Options -----------------------------------------------------------------
option(SPALL_BUILD_UI "Build the Qt Widgets front-end" ON)
option(SPALL_BUILD_TESTS "Build the GUI-less smoke test" ON)

# --- Output layout -----------------------------------------------------------
# Everything lands in <build>/bin/<Config> so the Debug and Release artifacts of
# the same name never collide. This matters for multi-config generators: if both
# configurations write SpAll_smoke.exe to the same path, the incremental linker
# can mix Debug objects (/MDd) with Release ones (/MD) and fail with
# LNK1319 "N mismatches detected".
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
# Per-config overrides must be cleared, otherwise they take precedence over the
# generator-expression form above and the collision comes back.
foreach(_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg})
    unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_cfg})
    unset(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_cfg})
endforeach()

# --- Warnings shared by our own targets --------------------------------------
add_library(spall_project_options INTERFACE)
if(MSVC)
    # /utf-8     : sources contain non-ASCII comments and string literals
    # /permissive-: conformance mode, no compiler-specific laxity
    # /W4        : requested warning level; /EHsc for standard C++ exceptions
    target_compile_options(spall_project_options INTERFACE
        /utf-8 /permissive- /W4 /EHsc
        $<$<CONFIG:Release>:/Gy>)
    target_compile_definitions(spall_project_options INTERFACE
        NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
else()
    target_compile_options(spall_project_options INTERFACE -Wall -Wextra -Wpedantic)
    target_compile_definitions(spall_project_options INTERFACE
        $<$<CONFIG:Release>:NDEBUG>)
endif()

# --- Dependencies ------------------------------------------------------------
include(cmake/Dependencies.cmake)

# --- Subdirectories ----------------------------------------------------------
# SPALL_BUILD_UI=OFF builds only the core library and the smoke test, so the
# GUI-less half of the project can be validated without Qt.
add_subdirectory(core)
if(SPALL_BUILD_UI)
    add_subdirectory(ui)
    add_subdirectory(app)
endif()
if(SPALL_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# --- Summary -----------------------------------------------------------------
message(STATUS "")
message(STATUS "SpAll ${PROJECT_VERSION} configuration summary")
message(STATUS "  generator        : ${CMAKE_GENERATOR}")
message(STATUS "  build type       : ${CMAKE_BUILD_TYPE}${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "  C++ standard     : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt6              : ${Qt6_VERSION} (${Qt6_DIR})")
message(STATUS "  spirv-opt        : ${SPALL_HAVE_OPT}")
message(STATUS "  deps dir (cache) : ${GLSL_TOOLS_DEPS_DIR}")
message(STATUS "  output dir       : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "  tests            : ${SPALL_BUILD_TESTS}")
message(STATUS "")
