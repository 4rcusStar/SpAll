# SpAll core — compilation, disassembly and reflection.
#
# This library has no Qt Widgets dependency (QtCore only), so it can be built and
# exercised without a GUI; see tests/SpAll_smoke.

set(SPALL_CORE_SOURCES
    ShaderStage.cpp
    Diagnostic.cpp
    CompileOptions.cpp
    ReflectData.cpp
    IncludePreprocessor.cpp
    GlslCompiler.cpp
    SpirvDisassembler.cpp
    SpirvReflection.cpp)

set(SPALL_CORE_HEADERS
    ShaderStage.h
    Diagnostic.h
    CompileOptions.h
    CompileResult.h
    ReflectData.h
    IncludePreprocessor.h
    GlslCompiler.h
    SpirvDisassembler.h
    SpirvReflection.h)

add_library(SpAll-core STATIC ${SPALL_CORE_SOURCES} ${SPALL_CORE_HEADERS})
add_library(SpAll::Core ALIAS SpAll-core)

target_include_directories(SpAll-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

# Qt6::Core only — never Qt6::Widgets (enforced by review, and by tests/ building
# against this target alone).
target_link_libraries(SpAll-core
    PUBLIC
        Qt6::Core
        spall_project_options
        # PUBLIC because SpirvReflection.h is a public header that includes
        # spirv_reflect.h, so consumers need that include directory too.
        spirv_reflect
    PRIVATE
        spall_spirv_headers
        ${SPALL_GLSLANG_TARGETS}
        SPIRV-Tools)

if(SPALL_HAVE_OPT AND TARGET SPIRV-Tools-opt)
    target_link_libraries(SpAll-core PRIVATE SPIRV-Tools-opt)
    # PUBLIC: consumers (the UI and the smoke test) need to know whether the
    # optimizer is available so they can grey out / skip optimization.
    target_compile_definitions(SpAll-core PUBLIC SPALL_HAVE_SPIRV_OPT=1)
else()
    target_compile_definitions(SpAll-core PUBLIC SPALL_HAVE_SPIRV_OPT=0)
endif()

# glslang's own CMakeLists adds its directories globally, and some of those names
# ("glslang", "SPIRV") are generic enough to shadow unrelated headers, so they are
# added here as SYSTEM instead. Both layouts are covered: newer glslang exposes
# headers as <glslang/...>, this SDK snapshot uses <glslang/Public/...> plus a
# top-level SPIRV/ directory.
target_include_directories(SpAll-core SYSTEM PRIVATE
    "${glslang_SOURCE_DIR}"
    "${glslang_SOURCE_DIR}/glslang"
    "${spirv_tools_SOURCE_DIR}/include"
    "${spirv_headers_SOURCE_DIR}/include")

if(WIN32)
    # glslang/spirv-tools pull in Win32 APIs.
    target_link_libraries(SpAll-core PRIVATE advapi32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(SpAll-core PRIVATE Threads::Threads)
endif()

set_target_properties(SpAll-core PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
