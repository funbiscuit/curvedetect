if( WIN32 )
    set(QT_WIN_PATH "C:\\" CACHE STRING "Path to installed qt binaries")
    message(STATUS "Looking for installed Qt5 at the following path:\n" ${QT_WIN_PATH})
    set(CMAKE_PREFIX_PATH ${QT_WIN_PATH})
endif()

find_package(Qt5 COMPONENTS Core Widgets Svg REQUIRED)

add_executable(curvedetect_gui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
        )

# sources for gui
target_sources(curvedetect_gui PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/clipboard.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/clipboard-nix.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/clipboard-win.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/curve_detect.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/image.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/image_elements.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/main_app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/curveview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/mat_file_writer.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/snap_cache.cpp
        )

# headers for moc generation
target_sources(curvedetect_gui PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/fileentrypopup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/curveview.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/mainwindow.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/spaceview.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/statusview.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/customstyle.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/inotifydialog.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/include/logdialog.h
        )

target_compile_features(curvedetect_gui PUBLIC cxx_std_11)
set_target_properties(curvedetect_gui PROPERTIES
        CXX_EXTENSIONS OFF
        AUTOMOC ON)

target_include_directories(curvedetect_gui
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_link_libraries(curvedetect_gui PRIVATE
        Qt5::Widgets Qt5::Svg portable_file_dialogs
        CurvedetectRes::CurvedetectRes
        )

if( WIN32 )
    option(WIN32_CONSOLE "Enable default windows console" OFF)
    # On Windows disable console if not needed
    if(NOT ${WIN32_CONSOLE})
        message(STATUS "Console is disabled")
        if (MSVC) # msvc compiler
            set_target_properties(curvedetect_gui PROPERTIES
                    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
                    )
        else() # gcc compiler
            set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mwindows")
        endif()
    endif()
endif()

install(TARGETS curvedetect_gui
        DESTINATION bin)
