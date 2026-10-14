# Copies the Qt runtime next to a built executable, with no external tools.
#
# Why not windeployqt: it spawns helper processes through pipes
# ("Unable to query qtpaths: Error running binary qtpaths: pipe: ..."), which is
# refused in restricted environments. Copying the handful of DLLs and plugins a
# Qt Widgets application needs is trivial and completely reliable, so this script
# does that with CMake's own file operations.
#
# Invoked at build time via `cmake -P deploy_qt.cmake` with -D arguments:
#   DEPLOY_QT_BIN_DIR  : Qt's bin directory (holds the Qt6*.dll files)
#   DEPLOY_QT_PLUGIN_DIR : Qt's plugins directory
#   DEPLOY_TARGET_DIR  : directory of the executable being deployed
#   DEPLOY_CONFIG      : Debug or Release (picks the 'd'-suffixed binaries)
#   DEPLOY_QT_DLLS     : semicolon list of Qt6*.dll to copy
#   DEPLOY_QT_PLUGINS  : semicolon list of plugin subdirectories to copy
#   DEPLOY_QM_FILES    : optional semicolon list of compiled translation catalogues to
#                        copy into translations/ next to the executable

if(NOT DEPLOY_TARGET_DIR)
    message(FATAL_ERROR "deploy_qt.cmake: DEPLOY_TARGET_DIR is not set")
endif()
if(NOT DEPLOY_QT_BIN_DIR OR NOT EXISTS "${DEPLOY_QT_BIN_DIR}")
    message(FATAL_ERROR
        "deploy_qt.cmake: Qt bin directory not found: '${DEPLOY_QT_BIN_DIR}'.\n"
        "What to do: configure with -DCMAKE_PREFIX_PATH=<Qt>/<version>/msvc2022_64, "
        "or build with -DSPALL_DEPLOY_QT=OFF and put Qt's bin directory on PATH.")
endif()

# On Windows a Debug build links the debug Qt DLLs, which carry a 'd' suffix
# (Qt6Cored.dll). Matching the configuration avoids mixing runtimes.
set(_suffix "")
if(DEPLOY_CONFIG STREQUAL "Debug")
    set(_suffix "d")
endif()

set(_copied 0)
set(_missing "")

foreach(_dll IN LISTS DEPLOY_QT_DLLS)
    # Turn Qt6Core.dll into Qt6Cored.dll for Debug.
    string(REGEX REPLACE "\\.dll$" "${_suffix}.dll" _name "${_dll}")
    set(_src "${DEPLOY_QT_BIN_DIR}/${_name}")
    if(NOT EXISTS "${_src}")
        list(APPEND _missing "${_name}")
        continue()
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_src}" "${DEPLOY_TARGET_DIR}/${_name}"
        RESULT_VARIABLE _result)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "deploy_qt.cmake: failed to copy ${_name} (exit ${_result})")
    endif()
    math(EXPR _copied "${_copied} + 1")
endforeach()

# Plugins. platforms/qwindows.dll is mandatory: without it Qt aborts at startup
# with "This application failed to start because no Qt platform plugin could be
# initialized". The others are conveniences (native style, common image formats).
#
# The list is explicit rather than a glob on purpose. Debug Qt plugins are named
# <name>d.dll, but several release plugins also end in 'd' (qdirect2d, qminimal,
# ...), so globbing "*d.dll" for Debug would drag in release binaries. Naming each
# plugin removes the ambiguity.
foreach(_plugin IN LISTS DEPLOY_QT_PLUGINS)
    get_filename_component(_dir "${_plugin}" DIRECTORY)
    get_filename_component(_base "${_plugin}" NAME_WE)
    if(_suffix STREQUAL "d")
        set(_name "${_base}d.dll")
    else()
        set(_name "${_base}.dll")
    endif()

    set(_src "${DEPLOY_QT_PLUGIN_DIR}/${_dir}/${_name}")
    if(NOT EXISTS "${_src}")
        list(APPEND _missing "${_dir}/${_name}")
        continue()
    endif()
    file(MAKE_DIRECTORY "${DEPLOY_TARGET_DIR}/${_dir}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${_src}" "${DEPLOY_TARGET_DIR}/${_dir}/${_name}"
        RESULT_VARIABLE _result)
    if(NOT _result EQUAL 0)
        message(FATAL_ERROR
            "deploy_qt.cmake: failed to copy plugin ${_dir}/${_name} (exit ${_result})")
    endif()
    math(EXPR _copied "${_copied} + 1")
endforeach()

if(_missing)
    message(WARNING
        "deploy_qt.cmake: these Qt libraries were not found in ${DEPLOY_QT_BIN_DIR} and were "
        "skipped: ${_missing}")
endif()

# Translation catalogues. They go into translations/ beside the executable, which is the
# first place Translator::searchPaths() looks. Missing catalogues are not an error: the
# application falls back to English, its source language.
if(DEPLOY_QM_FILES)
    set(_qm_dir "${DEPLOY_TARGET_DIR}/translations")
    file(MAKE_DIRECTORY "${_qm_dir}")
    foreach(_qm IN LISTS DEPLOY_QM_FILES)
        if(NOT EXISTS "${_qm}")
            message(WARNING
                "deploy_qt.cmake: translation catalogue not found, skipped: ${_qm}\n"
                "What to do: build the SpAll-translations target (lrelease) first.")
            continue()
        endif()
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_qm}" "${_qm_dir}"
            RESULT_VARIABLE _result)
        if(NOT _result EQUAL 0)
            message(FATAL_ERROR "deploy_qt.cmake: failed to copy ${_qm} (exit ${_result})")
        endif()
        math(EXPR _copied "${_copied} + 1")
    endforeach()
endif()

message(STATUS "deploy_qt: ${_copied} file(s) copied into ${DEPLOY_TARGET_DIR}")
