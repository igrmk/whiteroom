cmake_minimum_required(VERSION 3.13)

project(whiteroom LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    # setup RPATH but not RUNPATH
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--disable-new-dtags")
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--disable-new-dtags")

    set(CMAKE_INSTALL_RPATH ".")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
    set(CMAKE_SKIP_BUILD_RPATH TRUE)
endif()

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../bundle)

# simplify foo/../bar paths otherwise fixup_bundle throws the error on mac
get_filename_component(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} ABSOLUTE)

set(LOCAL_LIBS "${CMAKE_SOURCE_DIR}/../libs")

set(SOURCE_FILES
    main.cpp
    rtsp.cpp
    controller.cpp
    generator.cpp
    audio_output.cpp
    qml.qrc)

set(HEADER_FILES
    controller.h
    stateenum.h
    rtsp.h
    camera.h
    generator.h
    audio_output.h
    audio.h
    log.h
    audio_config.h
    audio_config_data.h
    white_math.h
    shared_amplitudes.h)

find_package(Qt5 COMPONENTS Core Quick Multimedia QuickControls2 REQUIRED)
if(Qt5_FOUND)
    message(STATUS "Found Qt ${Qt5_VERSION}: ${_qt5Core_install_prefix}")
endif()

if(UNIX AND NOT APPLE)
    find_package(ALSA REQUIRED)
endif()

link_directories(${LOCAL_LIBS}/portaudio-build/lib)
link_directories(${LOCAL_LIBS}/ffmpeg-build/lib)
if(WIN32)
    link_directories(${LOCAL_LIBS}/portaudio-build/bin)
    link_directories(${LOCAL_LIBS}/ffmpeg-build/bin)
endif()

if(NOT APPLE)
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
else()
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES} ${HEADER_FILES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_LIBS}/portaudio-build/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_LIBS}/ffmpeg-build/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LOCAL_LIBS}/spdlog/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME} PRIVATE
    avformat
    swscale
    avcodec
    avutil
    pthread
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
    portaudio
    portaudiocpp)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ALSA_LIBRARIES} jack)
endif()

if(APPLE)
    set(RUNTIME_DIR "whiteroom.app/Contents/MacOS")
else()
    set(RUNTIME_DIR ".")
endif()

install(
    TARGETS whiteroom
    BUNDLE DESTINATION .
    COMPONENT runtime
    RUNTIME DESTINATION "${RUNTIME_DIR}")

include(InstallRequiredSystemLibraries)

if(APPLE)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "[Paths]\nPlugins = Plugins\nImports = Resources\nQml2Imports = Resources")

    set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Resources")
    set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" DESTINATION "${RESOURCES_DIR}")
    install(FILES "${_qt5Core_install_prefix}/plugins/platforms/libqcocoa.dylib" DESTINATION "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/Platforms")

    function ("install_qml_plugin" REL_DIR LIB GOBACK)
        message(STATUS "Installing QML plugin ${_qt5Core_install_prefix}/qml/${REL_DIR}/${LIB} to ${PLUGINS_DIR}")
        install(FILES "${_qt5Core_install_prefix}/qml/${REL_DIR}/${LIB}" DESTINATION "${PLUGINS_DIR}")
        install(FILES "${_qt5Core_install_prefix}/qml/${REL_DIR}/qmldir" DESTINATION "${RESOURCES_DIR}/${REL_DIR}")
        install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink \"${GOBACK}/Plugins/${LIB}\" \"${RESOURCES_DIR}/${REL_DIR}/${LIB}\")")
    endfunction()

    install_qml_plugin("QtQuick.2" "libqtquick2plugin.dylib" "../..")
    install_qml_plugin("QtQuick/Window.2" "libwindowplugin.dylib" "../../..")
    install_qml_plugin("QtQuick/Layouts" "libqquicklayoutsplugin.dylib" "../../..")
    install_qml_plugin("QtQuick/Controls.2" "libqtquickcontrols2plugin.dylib" "../../..")
    install_qml_plugin("QtQuick/Controls.2/Material" "libqtquickcontrols2materialstyleplugin.dylib" "../../../..")
    install_qml_plugin("QtQuick/Templates.2" "libqtquicktemplates2plugin.dylib" "../../..")
elseif(WIN32)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "[Paths]\nPlugins = Plugins\nImports = Resources\nQml2Imports = Resources")
    set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/Resources")
    set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/Plugins")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}")
    install(FILES "${_qt5Core_install_prefix}/share/qt5/plugins/platforms/qwindows.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}/platforms")

    function ("install_qml_plugin" REL_DIR LIB)
        message(STATUS "Installing QML plugin ${_qt5Core_install_prefix}/share/qt5/qml/${REL_DIR}/${LIB} to ${PLUGINS_DIR}")
        install(FILES "${_qt5Core_install_prefix}/share/qt5/qml/${REL_DIR}/${LIB}" DESTINATION "${PLUGINS_DIR}")
        install(FILES "${_qt5Core_install_prefix}/share/qt5/qml/${REL_DIR}/${LIB}" DESTINATION "${RESOURCES_DIR}/${REL_DIR}")
        install(FILES "${_qt5Core_install_prefix}/share/qt5/qml/${REL_DIR}/qmldir" DESTINATION "${RESOURCES_DIR}/${REL_DIR}")
    endfunction()

    install_qml_plugin("QtQuick.2"                   "qtquick2plugin.dll")
    install_qml_plugin("QtQuick/Window.2"            "windowplugin.dll")
    install_qml_plugin("QtQuick/Layouts"             "qquicklayoutsplugin.dll")
    install_qml_plugin("QtQuick/Controls.2"          "qtquickcontrols2plugin.dll")
    install_qml_plugin("QtQuick/Controls.2/Material" "qtquickcontrols2materialstyleplugin.dll")
    install_qml_plugin("QtQuick/Templates.2"         "qtquicktemplates2plugin.dll")
endif()

install(CODE [[
    set(LOCAL_LIBS "${CMAKE_SOURCE_DIR}/../libs")
    if(WIN32)
        set(BUNDLE "${CMAKE_INSTALL_PREFIX}/whiteroom.exe")
    elseif(APPLE)
        set(BUNDLE "${CMAKE_INSTALL_PREFIX}/whiteroom.app")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        set(LIBS
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/Platforms/libqcocoa.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libqtquick2plugin.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libwindowplugin.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libqquicklayoutsplugin.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libqtquickcontrols2plugin.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libqtquickcontrols2materialstyleplugin.dylib"
            "${CMAKE_INSTALL_PREFIX}/whiteroom.app/Contents/Plugins/libqtquicktemplates2plugin.dylib"
        )
    else()
        set(BUNDLE "${CMAKE_INSTALL_PREFIX}/whiteroom")
    endif()

    include(BundleUtilities)

    set(DIRS "${LOCAL_LIBS}/ffmpeg-build/lib;${LOCAL_LIBS}/portaudio-build/lib")
    if(WIN32)
        set(DIRS "$DIRS;${LOCAL_LIBS}/ffmpeg-build/bin;${LOCAL_LIBS}/portaudio-build/bin")
        set(LIBS
            "${CMAKE_INSTALL_PREFIX}/platforms/qwindows.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/qtquick2plugin.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/windowplugin.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/qquicklayoutsplugin.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/qtquickcontrols2plugin.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/qtquickcontrols2materialstyleplugin.dll"
            "${CMAKE_INSTALL_PREFIX}/Plugins/qtquicktemplates2plugin.dll"
        )
    endif()

    fixup_bundle("${BUNDLE}" "${LIBS}" "${DIRS}")
]])
