# Third-party dependency wiring for SpAll.
#
# Dependency order is load-bearing:
#     SPIRV-Headers -> SPIRV-Tools -> glslang
# glslang's top-level CMakeLists.txt contains
#     if(ENABLE_OPT AND NOT TARGET SPIRV-Tools-opt) message(SEND_ERROR ...)
# so SPIRV-Tools (which provides the SPIRV-Tools-opt target) MUST be made
# available before glslang. Reversing the order fails at configure time.
#
# Nothing here links by library file name. The Windows Debug postfix
# (glslangd.lib / SPIRV-Toolsd.lib) makes file-name linking silently wrong, so
# every consumer links CMake targets instead.
#
# Offline mode: put the sources in third_party/<name> (or point
# -DGLSL_TOOLS_DEPS_DIR=... at a directory containing them) and no network
# access is required.

include(FetchContent)

# --- Qt 6 (system install, never fetched) ------------------------------------
# Concurrent provides QtConcurrent, used to run compiles off the GUI thread.
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Gui Core Concurrent)

# --- SPIR-V / glslang source locations ---------------------------------------
set(GLSL_TOOLS_DEPS_DIR "${CMAKE_SOURCE_DIR}/third_party" CACHE PATH
    "Local root directory holding pre-fetched dependency sources")

set(SPALL_DEPS_FETCH_TAGS ON CACHE BOOL
    "Allow downloading dependencies over the network when no local copy exists")

# Pinned revisions. GIT_TAG may be a tag, branch or commit hash.
#
# All four dependencies use the SAME Vulkan SDK tag on purpose. The versions
# named in the original specification (glslang 15.3.0 together with
# SPIRV-Tools vulkan-sdk-1.4.304.0) do not actually build: glslang 15.3.0's
# SPIRV/SpvTools.cpp calls spvValidatorOptionsSetAllowOffsetTextureOperand(),
# which does not exist in SPIRV-Tools 1.4.304.0's libspirv.h. Using one SDK tag
# for the whole set keeps glslang and SPIRV-Tools on the combination Khronos
# actually ships and tests together.
set(SPALL_SPIRV_HEADERS_TAG "vulkan-sdk-1.4.304.0" CACHE STRING "SPIRV-Headers revision")
set(SPALL_SPIRV_TOOLS_TAG   "vulkan-sdk-1.4.304.0" CACHE STRING "SPIRV-Tools revision")
set(SPALL_GLSLANG_TAG       "vulkan-sdk-1.4.304.0" CACHE STRING "glslang revision")
set(SPALL_SPIRV_REFLECT_TAG "vulkan-sdk-1.4.304.0" CACHE STRING "SPIRV-Reflect revision")

# Declare each dependency as either a local source tree or a git fetch. Passing
# SOURCE_DIR to FetchContent_Declare while disabling updates makes the local
# case fully offline.
#
# The repository URL is passed in explicitly rather than derived from the
# FetchContent name: the upstream repositories are "SPIRV-Headers",
# "SPIRV-Tools", "glslang" and "SPIRV-Reflect", and GitHub is case- and
# hyphen-sensitive (a lowercased "spirv_headers.git" does not exist, which
# surfaces as a confusing "could not read Username" prompt).
function(spall_declare_dependency name repo tag)
    set(_local "${GLSL_TOOLS_DEPS_DIR}/${name}")
    if(EXISTS "${_local}/CMakeLists.txt" OR EXISTS "${_local}/include/spirv/unified1/spirv.h"
       OR EXISTS "${_local}/spirv_reflect.c")
        message(STATUS "SpAll: using local dependency source ${_local}")
        FetchContent_Declare(${name}
            SOURCE_DIR "${_local}"
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND "")
    elseif(NOT SPALL_DEPS_FETCH_TAGS)
        message(FATAL_ERROR
            "Dependency '${name}' is not present in ${GLSL_TOOLS_DEPS_DIR} and "
            "network fetching is disabled (SPALL_DEPS_FETCH_TAGS=OFF).\n"
            "What to do: place the ${name} sources at ${_local}, or reconfigure "
            "with -DSPALL_DEPS_FETCH_TAGS=ON to download them.")
    else()
        message(STATUS "SpAll: fetching ${repo} @ ${tag}")
        FetchContent_Declare(${name}
            GIT_REPOSITORY "https://github.com/KhronosGroup/${repo}.git"
            GIT_TAG "${tag}"
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE)
    endif()
endfunction()

# --- Third-party build options ----------------------------------------------
# Keep only static libraries: no tests, no tools, no install rules, no shared libs.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(SPIRV_SKIP_TESTS      ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_INSTALL    ON CACHE BOOL "" FORCE)
set(SPIRV_TOOLS_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_SPIRV_TOOLS_INSTALL OFF CACHE BOOL "" FORCE)

set(GLSLANG_TESTS           OFF CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL  OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL             OFF CACHE BOOL "" FORCE)

# --- 1. SPIRV-Headers (headers only) ----------------------------------------
spall_declare_dependency(spirv_headers "SPIRV-Headers" "${SPALL_SPIRV_HEADERS_TAG}")
FetchContent_MakeAvailable(spirv_headers)

# Normalize the include directory under one name so consumers never guess.
add_library(spall_spirv_headers INTERFACE)
target_include_directories(spall_spirv_headers SYSTEM INTERFACE
    "${spirv_headers_SOURCE_DIR}/include")

# --- 2. SPIRV-Tools (library + optimizer) -----------------------------------
spall_declare_dependency(spirv_tools "SPIRV-Tools" "${SPALL_SPIRV_TOOLS_TAG}")
set(SPIRV-Headers_SOURCE_DIR "${spirv_headers_SOURCE_DIR}" CACHE PATH "" FORCE)
FetchContent_MakeAvailable(spirv_tools)

if(TARGET SPIRV-Tools-opt)
    set(SPALL_HAVE_OPT ON)
else()
    # Without SPIRV-Tools-opt the optimizer cannot be linked. Disable glslang's
    # dependency on it rather than letting glslang's CMakeLists SEND_ERROR, and
    # let the UI grey out its optimization controls.
    set(SPALL_HAVE_OPT OFF)
    set(ENABLE_OPT OFF CACHE BOOL "" FORCE)
    message(WARNING
        "SpAll: SPIRV-Tools-opt target is unavailable, so SPIR-V optimization is "
        "disabled. What to do: ensure the SPIRV-Tools sources are complete (they "
        "must include the 'opt' directory) and reconfigure.")
endif()

# --- 3. glslang (must come after SPIRV-Tools) -------------------------------
spall_declare_dependency(glslang "glslang" "${SPALL_GLSLANG_TAG}")
set(ENABLE_OPT ${SPALL_HAVE_OPT} CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glslang)

# Collect the glslang targets that actually exist. Names differ slightly across
# glslang releases, so probe instead of assuming.
set(SPALL_GLSLANG_TARGETS "")
foreach(_tgt glslang::glslang glslang::MachineIndependent glslang::GenericCodeGen
             glslang::SPIRV glslang::OSDependent glslang::OGLCompiler
             glslang::HLSL glslang::SPVRemapper glslang::SPIRV-Tools-opt)
    if(TARGET ${_tgt})
        list(APPEND SPALL_GLSLANG_TARGETS ${_tgt})
    endif()
endforeach()

# GetDefaultResources() lives in the separate default-resource-limits library.
# Without it the link fails with an unresolved external for GetDefaultResources.
if(TARGET glslang-default-resource-limits)
    list(APPEND SPALL_GLSLANG_TARGETS glslang-default-resource-limits)
elseif(TARGET glslang::glslang-default-resource-limits)
    list(APPEND SPALL_GLSLANG_TARGETS glslang::glslang-default-resource-limits)
endif()
list(REMOVE_DUPLICATES SPALL_GLSLANG_TARGETS)

if(NOT TARGET glslang::SPIRV)
    message(FATAL_ERROR
        "glslang::SPIRV target was not created. What to do: check that the glslang "
        "sources at ${glslang_SOURCE_DIR} are complete and reconfigure.")
endif()
if(SPALL_HAVE_OPT AND TARGET glslang::SPIRV-Tools-opt)
    list(APPEND SPALL_GLSLANG_TARGETS glslang::SPIRV-Tools-opt)
endif()
list(REMOVE_DUPLICATES SPALL_GLSLANG_TARGETS)

message(STATUS "SpAll: glslang targets -> ${SPALL_GLSLANG_TARGETS}")

# --- 4. SPIRV-Reflect (source only, no exported CMake target) ---------------
# SPIRV-Reflect ships no CMake project, so compile its single translation unit
# directly. It is downloaded with a plain clone because FetchContent_Populate is
# deprecated for this purpose; the local-source path above is preferred.
spall_declare_dependency(spirv_reflect "SPIRV-Reflect" "${SPALL_SPIRV_REFLECT_TAG}")
FetchContent_MakeAvailable(spirv_reflect)

if(NOT EXISTS "${spirv_reflect_SOURCE_DIR}/spirv_reflect.c")
    message(FATAL_ERROR
        "SPIRV-Reflect source spirv_reflect.c not found under "
        "${spirv_reflect_SOURCE_DIR}.\n"
        "What to do: place the SPIRV-Reflect sources at "
        "${GLSL_TOOLS_DEPS_DIR}/spirv_reflect and reconfigure.")
endif()

add_library(spirv_reflect STATIC "${spirv_reflect_SOURCE_DIR}/spirv_reflect.c")
target_include_directories(spirv_reflect SYSTEM PUBLIC
    "${spirv_reflect_SOURCE_DIR}"
    "${spirv_headers_SOURCE_DIR}/include")
# The bundled spirv.h is used; do not redirect to a system one.
target_compile_definitions(spirv_reflect PRIVATE SPIRV_REFLECT_USE_SYSTEM_SPIRV_H=0)
set_target_properties(spirv_reflect PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    # SPIRV-Reflect is third-party C: keep our warning flags off it.
    target_compile_options(spirv_reflect PRIVATE /W0)
else()
    target_compile_options(spirv_reflect PRIVATE -w)
endif()

message(STATUS "SpAll: spirv_reflect sources -> ${spirv_reflect_SOURCE_DIR}")
