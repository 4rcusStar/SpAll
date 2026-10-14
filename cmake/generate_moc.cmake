# Runs Qt's moc over the SpAll UI headers without AUTOMOC.
#
# Why this exists: CMake's AUTOMOC drives moc through a libuv-based process spawn
# (`cmake -E cmake_autogen`), which fails in this sandbox with
# "libuv process spawn failed: operation not permitted". Running moc.exe directly
# through execute_process() works, so this script does exactly that.
#
# It is invoked at build time via `cmake -P generate_moc.cmake`, so the moc
# sources stay current with the headers.
#
# Required variables (passed with -D on the command line):
#   MOC_EXECUTABLE : path to Qt's moc.exe
#   MOC_OUTPUT_DIR : directory for the generated moc_*.cpp files
#   MOC_HEADERS    : semicolon-separated list of headers containing Q_OBJECT
#   MOC_INCLUDES   : semicolon-separated list of -I include directories

if(NOT MOC_EXECUTABLE)
    message(FATAL_ERROR "generate_moc.cmake: MOC_EXECUTABLE is not set")
endif()
if(NOT MOC_OUTPUT_DIR)
    message(FATAL_ERROR "generate_moc.cmake: MOC_OUTPUT_DIR is not set")
endif()
if(NOT EXISTS "${MOC_EXECUTABLE}")
    message(FATAL_ERROR
        "generate_moc.cmake: moc not found at ${MOC_EXECUTABLE}.\n"
        "What to do: re-run tools/bootstrap_qt.ps1, or configure with "
        "-DCMAKE_PREFIX_PATH pointing at a complete Qt 6 installation.")
endif()

file(MAKE_DIRECTORY "${MOC_OUTPUT_DIR}")

set(_include_args "")
foreach(_dir IN LISTS MOC_INCLUDES)
    if(_dir)
        list(APPEND _include_args "-I${_dir}")
    endif()
endforeach()

set(_generated 0)
set(_skipped 0)

foreach(_header IN LISTS MOC_HEADERS)
    if(NOT EXISTS "${_header}")
        message(WARNING "generate_moc.cmake: header not found: ${_header}")
        continue()
    endif()

    get_filename_component(_name "${_header}" NAME_WE)
    set(_output "${MOC_OUTPUT_DIR}/moc_${_name}.cpp")

    # Regenerate only when the header is newer than the generated file.
    if(EXISTS "${_output}")
        file(TIMESTAMP "${_header}" _header_time "%Y%m%d%H%M%S")
        file(TIMESTAMP "${_output}" _output_time "%Y%m%d%H%M%S")
        if(_header_time STREQUAL _output_time OR _header_time LESS _output_time)
            math(EXPR _skipped "${_skipped} + 1")
            continue()
        endif()
    endif()

    execute_process(
        COMMAND "${MOC_EXECUTABLE}" ${_include_args} -o "${_output}" "${_header}"
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)

    if(NOT _result EQUAL 0)
        message(FATAL_ERROR
            "generate_moc.cmake: moc failed for ${_header} (exit ${_result}).\n"
            "stdout: ${_stdout}\nstderr: ${_stderr}")
    endif()
    math(EXPR _generated "${_generated} + 1")
endforeach()

message(STATUS "generate_moc: ${_generated} regenerated, ${_skipped} up to date")
