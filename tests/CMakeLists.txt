# SpAll smoke test
#
# GUI-less acceptance test over examples/. Links only SpAll-core, which proves
# the core library has no Qt Widgets dependency.

add_executable(SpAll_smoke smoke_main.cpp)
target_link_libraries(SpAll_smoke PRIVATE SpAll-core)

set_target_properties(SpAll_smoke PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # Console output must survive; keep the console subsystem (the default for
    # add_executable) so the test prints to the terminal.
    target_compile_options(SpAll_smoke PRIVATE /W4)
endif()

# Make `ctest` able to run it from the build tree.
enable_testing()
add_test(NAME SpAll_smoke
         COMMAND SpAll_smoke
         WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_tests_properties(SpAll_smoke PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# Development helper: dumps raw reflection data for the bundled examples, which
# makes mismatches between expectations and SPIRV-Reflect's output obvious.
option(SPALL_BUILD_REFLECT_DUMP "Build the reflection dump helper" OFF)
if(SPALL_BUILD_REFLECT_DUMP)
    add_executable(SpAll_reflect_dump reflect_dump_main.cpp)
    target_link_libraries(SpAll_reflect_dump PRIVATE SpAll-core)
    set_target_properties(SpAll_reflect_dump PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Regression test for the pre-compile syntax hints (missing-semicolon detection).
# It reproduces the checker's rules so the reported false positives stay covered.
option(SPALL_BUILD_HINT_CHECK "Build the syntax-hint regression test" OFF)
if(SPALL_BUILD_HINT_CHECK)
    add_executable(SpAll_hint_check hint_check_main.cpp)
    target_link_libraries(SpAll_hint_check PRIVATE SpAll-core)
    set_target_properties(SpAll_hint_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Verifies the file browser offers exactly the extensions the compiler understands
# and that every one of them maps back to a pipeline stage.
option(SPALL_BUILD_FILE_BROWSER_CHECK "Build the file browser test" OFF)
if(SPALL_BUILD_FILE_BROWSER_CHECK)
    add_executable(SpAll_file_browser_check file_browser_check_main.cpp)
    target_link_libraries(SpAll_file_browser_check PRIVATE SpAll::Ui)
    set_target_properties(SpAll_file_browser_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Verifies a saved .spv file is a valid module: correct size, little-endian magic,
# binary (not text-mode) content, and still passing validation after a round trip.
option(SPALL_BUILD_SPV_SAVE_CHECK "Build the .spv save test" OFF)
if(SPALL_BUILD_SPV_SAVE_CHECK)
    add_executable(SpAll_spv_save_check spv_save_check_main.cpp)
    target_link_libraries(SpAll_spv_save_check PRIVATE SpAll-core)
    set_target_properties(SpAll_spv_save_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Drives completion through EditorTab (the widget the application actually uses)
# rather than a bare editor, so widget-level regressions are caught.
option(SPALL_BUILD_COMPLETION_GUI_CHECK "Build the EditorTab completion test" OFF)
if(SPALL_BUILD_COMPLETION_GUI_CHECK)
    add_executable(SpAll_completion_gui_check completion_gui_check_main.cpp)
    target_link_libraries(SpAll_completion_gui_check PRIVATE SpAll::Ui)
    set_target_properties(SpAll_completion_gui_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Verifies the code-completion popup actually appears and that Tab accepts a
# candidate. The popup silently did nothing before, so this is worth locking down.
option(SPALL_BUILD_COMPLETION_CHECK "Build the completion popup test" OFF)
if(SPALL_BUILD_COMPLETION_CHECK)
    add_executable(SpAll_completion_check completion_check_main.cpp)
    target_link_libraries(SpAll_completion_check PRIVATE SpAll::Ui)
    set_target_properties(SpAll_completion_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Regression test for the diagnostic line tints: verifies the foreground and
# background of every tint actually have enough contrast, and that syntax
# highlighting no longer clobbers a tinted line's background.
option(SPALL_BUILD_CONTRAST_CHECK "Build the diagnostic tint contrast test" OFF)
if(SPALL_BUILD_CONTRAST_CHECK)
    add_executable(SpAll_contrast_check contrast_check_main.cpp)
    target_link_libraries(SpAll_contrast_check PRIVATE SpAll::Ui)
    set_target_properties(SpAll_contrast_check PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()

# Helper for the §9.4 cross-validation against the Vulkan SDK's spirv-dis:
# it writes the module plus two disassemblies to disk.
option(SPALL_BUILD_CROSS_VALIDATE "Build the spirv-dis cross-validation helper" OFF)
if(SPALL_BUILD_CROSS_VALIDATE)
    add_executable(SpAll_cross_validate cross_validate_main.cpp)
    target_link_libraries(SpAll_cross_validate PRIVATE SpAll-core)
    set_target_properties(SpAll_cross_validate PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
endif()
