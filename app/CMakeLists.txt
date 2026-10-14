# SpAll — application entry point.

add_executable(SpAll WIN32 main.cpp)
target_link_libraries(SpAll PRIVATE SpAll::Ui spall_project_options)

set_target_properties(SpAll PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

# --- Qt runtime deployment ---------------------------------------------------
#
# ON by default so the built executable can be double-clicked. Qt is linked
# dynamically (the Qt binaries shipped by the installer are dynamic), so without
# this step launching the .exe directly fails with
# "cannot find Qt6Widgets.dll".
#
# The copy is done by cmake/deploy_qt.cmake rather than windeployqt, which spawns
# helper processes through pipes and fails in restricted environments.
option(SPALL_DEPLOY_QT "Copy the Qt runtime next to SpAll so it runs standalone" ON)

if(WIN32)
    # The Qt DLLs used by a Widgets application; verified with dumpbin /DEPENDENTS.
    # Qt6Concurrent is listed because the UI target links it for compilation even
    # though the linker drops it here.
    set(SPALL_QT_DLLS Qt6Core.dll Qt6Gui.dll Qt6Widgets.dll Qt6Concurrent.dll)
    # platforms/qwindows.dll is mandatory for the GUI to start; the rest provide
    # the native style and the usual image formats. Names are given without the
    # Debug 'd' suffix, which the deploy script adds when needed.
    set(SPALL_QT_PLUGINS
        platforms/qwindows
        platforms/qminimal
        platforms/qoffscreen
        styles/qmodernwindowsstyle
        imageformats/qico
        imageformats/qjpeg
        imageformats/qgif
        imageformats/qsvg)

    set(SPALL_QT_DIR  "${Qt6_DIR}/../../..")
    set(SPALL_QT_BIN  "${SPALL_QT_DIR}/bin")
    set(SPALL_QT_PLUGIN_DIR "${SPALL_QT_DIR}/plugins")

    if(SPALL_DEPLOY_QT)
        set(_deploy_script "${CMAKE_SOURCE_DIR}/cmake/deploy_qt.cmake")
        add_custom_command(TARGET SpAll POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                    "-DDEPLOY_QT_BIN_DIR=${SPALL_QT_BIN}"
                    "-DDEPLOY_QT_PLUGIN_DIR=${SPALL_QT_PLUGIN_DIR}"
                    "-DDEPLOY_TARGET_DIR=$<TARGET_FILE_DIR:SpAll>"
                    "-DDEPLOY_CONFIG=$<CONFIG>"
                    "-DDEPLOY_QT_DLLS=${SPALL_QT_DLLS}"
                    "-DDEPLOY_QT_PLUGINS=${SPALL_QT_PLUGINS}"
                    "-DDEPLOY_QM_FILES=${SPALL_QM_FILES}"
                    -P "${_deploy_script}"
            COMMENT "Deploying Qt runtime next to SpAll")
    endif()

    # Put Qt's bin directory on PATH for the Visual Studio debugger as well, so
    # launching from the IDE works regardless of the deployment setting.
    set_target_properties(SpAll PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${SPALL_QT_BIN};%PATH%")
endif()

# The executable lives in bin/<Config>; make the examples reachable from there so
# the left-hand file list is populated when running from the build tree.
add_custom_command(TARGET SpAll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/examples"
            "$<TARGET_FILE_DIR:SpAll>/examples"
    COMMENT "Copying examples next to SpAll")
