# SpAll UI — Qt Widgets front-end.
#
# All compilation/reflection logic lives in SpAll-core; this library only
# displays and interacts.
#
# Qt's metadata compiler (moc) is driven by a build-time script instead of
# AUTOMOC. CMake's AUTOMOC spawns moc through libuv (`cmake -E cmake_autogen`),
# which this sandbox refuses with "libuv process spawn failed: operation not
# permitted", while running moc.exe directly works. See cmake/generate_moc.cmake.

set(SPALL_UI_HEADERS
    MainWindow.h
    EditorTab.h
    LineNumberArea.h
    GlslHighlighter.h
    GlslCompleter.h
    FileBrowser.h
    SettingsDialog.h
    Translator.h
    StartupTrace.h
    panels/AsmPanel.h
    panels/ReflectPanel.h
    panels/DiagnosticsPanel.h
    panels/OutputPanel.h)

set(SPALL_UI_SOURCES
    MainWindow.cpp
    EditorTab.cpp
    LineNumberArea.cpp
    GlslHighlighter.cpp
    GlslCompleter.cpp
    GlslLanguageData.cpp
    FileBrowser.cpp
    SettingsDialog.cpp
    Translator.cpp
    StartupTrace.cpp
    panels/AsmPanel.cpp
    panels/ReflectPanel.cpp
    panels/DiagnosticsPanel.cpp
    panels/OutputPanel.cpp)

# Headers that actually contain Q_OBJECT (moc has nothing to do for the rest).
set(SPALL_UI_MOC_HEADERS
    MainWindow.h
    EditorTab.h
    LineNumberArea.h
    GlslHighlighter.h
    GlslCompleter.h
    FileBrowser.h
    SettingsDialog.h
    Translator.h
    panels/AsmPanel.h
    panels/ReflectPanel.h
    panels/DiagnosticsPanel.h
    panels/OutputPanel.h)

# Header-only source: GlslLanguageData carries no Q_OBJECT.
list(APPEND SPALL_UI_HEADERS GlslLanguageData.h)

# --- moc generation ----------------------------------------------------------
find_program(SPALL_MOC_EXECUTABLE
    NAMES moc moc.exe
    HINTS "${Qt6_DIR}/../../../bin"
    REQUIRED)

set(SPALL_UI_MOC_DIR "${CMAKE_CURRENT_BINARY_DIR}/moc")

set(_moc_headers_abs "")
foreach(_header IN LISTS SPALL_UI_MOC_HEADERS)
    list(APPEND _moc_headers_abs "${CMAKE_CURRENT_SOURCE_DIR}/${_header}")
endforeach()

set(_moc_includes
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/core"
    "${Qt6_DIR}/../../../include"
    "${Qt6_DIR}/../../../include/QtCore"
    "${Qt6_DIR}/../../../include/QtGui"
    "${Qt6_DIR}/../../../include/QtWidgets")

# Quoted as one argument each so paths with spaces survive.
string(REPLACE ";" "\\;" _moc_headers_arg "${_moc_headers_abs}")
string(REPLACE ";" "\\;" _moc_includes_arg "${_moc_includes}")

set(_moc_script "${CMAKE_SOURCE_DIR}/cmake/generate_moc.cmake")
set(_moc_command
    "${CMAKE_COMMAND}"
    "-DMOC_EXECUTABLE=${SPALL_MOC_EXECUTABLE}"
    "-DMOC_OUTPUT_DIR=${SPALL_UI_MOC_DIR}"
    "-DMOC_HEADERS=${_moc_headers_arg}"
    "-DMOC_INCLUDES=${_moc_includes_arg}"
    -P "${_moc_script}")

# Run at configure time so the sources exist before the target is generated.
execute_process(COMMAND ${_moc_command} RESULT_VARIABLE _moc_result)
if(NOT _moc_result EQUAL 0)
    message(FATAL_ERROR
        "Initial moc generation failed (exit ${_moc_result}).\n"
        "What to do: check that Qt's moc.exe runs, then re-run the configure step.")
endif()

# ...and again at build time, so editing a header regenerates its moc file.
add_custom_target(SpAll-ui-moc ALL
    COMMAND ${_moc_command}
    COMMENT "Generating Qt moc sources for SpAll-ui"
    VERBATIM)

set(_moc_sources "")
foreach(_header IN LISTS SPALL_UI_MOC_HEADERS)
    get_filename_component(_name "${_header}" NAME_WE)
    list(APPEND _moc_sources "${SPALL_UI_MOC_DIR}/moc_${_name}.cpp")
endforeach()

# --- target ------------------------------------------------------------------
add_library(SpAll-ui STATIC ${SPALL_UI_SOURCES} ${SPALL_UI_HEADERS} ${_moc_sources})
add_library(SpAll::Ui ALIAS SpAll-ui)

add_dependencies(SpAll-ui SpAll-ui-moc)

target_include_directories(SpAll-ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SPALL_UI_MOC_DIR})

target_link_libraries(SpAll-ui
    PUBLIC
        SpAll-core
        Qt6::Widgets
        Qt6::Gui
        Qt6::Core
        Qt6::Concurrent
        spall_project_options)

set_target_properties(SpAll-ui PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(WIN32)
    target_compile_definitions(SpAll-ui PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# --- translation catalogues --------------------------------------------------
#
# English is the source language, so it needs no catalogue: the tr() calls in the
# sources already contain it. Chinese ships as spall_zh_CN.ts, extracted by lupdate and
# compiled to spall_zh_CN.qm by lrelease, both driven by Qt's own LinguistTools package
# rather than a hand-rolled script.
#
# The catalogues are *not* embedded as a Qt resource. deploy_qt.cmake copies the .qm
# files into translations/ next to the executable, which is one of the locations
# Translator::searchPaths() probes, so a user can drop in an extra language without
# rebuilding.
find_package(Qt6 QUIET COMPONENTS LinguistTools)

if(Qt6LinguistTools_FOUND)
    set(SPALL_TRANSLATIONS_DIR "${CMAKE_SOURCE_DIR}/translations")
    set(SPALL_TS_FILES "${SPALL_TRANSLATIONS_DIR}/spall_zh_CN.ts")

    # The .ts must exist before qt_add_lrelease() runs lrelease over it, so seed an empty
    # catalogue on a fresh checkout. lupdate fills it in on the first build.
    if(NOT EXISTS "${SPALL_TS_FILES}")
        file(WRITE "${SPALL_TS_FILES}"
"<?xml version=\"1.0\" encoding=\"utf-8\"?>
<!DOCTYPE TS>
<TS version=\"2.1\" language=\"zh_CN\">
</TS>
")
        message(STATUS "  translations     : seeded ${SPALL_TS_FILES}")
    endif()

    # Keep the .ts in sync as part of the normal build. lupdate is given an explicit file
    # list because the project has no Qt AUTOMOC/AUTOUIC targets for it to discover
    # sources from. The translatable strings live in both ui/ and core/, so both are
    # listed here rather than relying on target introspection.
    set(SPALL_TRANSLATABLE_SOURCES
        ${SPALL_UI_SOURCES}
        ${SPALL_UI_HEADERS}
        ${CMAKE_SOURCE_DIR}/core/Diagnostic.cpp
        ${CMAKE_SOURCE_DIR}/core/Diagnostic.h
        ${CMAKE_SOURCE_DIR}/core/GlslCompiler.cpp
        ${CMAKE_SOURCE_DIR}/core/IncludePreprocessor.cpp
        ${CMAKE_SOURCE_DIR}/core/ReflectData.cpp
        ${CMAKE_SOURCE_DIR}/core/SpirvDisassembler.cpp
        ${CMAKE_SOURCE_DIR}/core/SpirvReflection.cpp
        ${CMAKE_SOURCE_DIR}/core/CompileOptions.cpp
        ${CMAKE_SOURCE_DIR}/app/main.cpp)

    qt_add_lupdate(TS_FILES ${SPALL_TS_FILES}
        SOURCES ${SPALL_TRANSLATABLE_SOURCES}
        LUPDATE_TARGET SpAll-lupdate)

    qt_add_lrelease(TS_FILES ${SPALL_TS_FILES}
        QM_FILES_OUTPUT_VARIABLE _spall_qm_files
        LRELEASE_TARGET SpAll-translations)

    # Promoted to a cache entry so app/CMakeLists.txt (a sibling directory scope) can pass
    # the list to the deployment script. A normal variable would not be visible there.
    set(SPALL_QM_FILES "${_spall_qm_files}" CACHE INTERNAL
        "Compiled translation catalogues to deploy next to the executable")

    message(STATUS "  translations     : ${SPALL_TRANSLATIONS_DIR}")
else()
    set(SPALL_QM_FILES "" CACHE INTERNAL
        "Compiled translation catalogues to deploy next to the executable")
    message(STATUS "  translations     : disabled (Qt6 LinguistTools not found)")
endif()
