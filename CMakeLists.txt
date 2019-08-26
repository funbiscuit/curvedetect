cmake_minimum_required(VERSION 3.13)
project(curvedetect)

set(CMAKE_CXX_STANDARD 11)

# build resources
find_package( PythonInterp 3.0 REQUIRED )
set(RES_GEN_COMMAND ${PYTHON_EXECUTABLE}
        ${PROJECT_SOURCE_DIR}/libs/resource-builder/resource-builder.py #path to python script that builds resources
        ${PROJECT_SOURCE_DIR}/res   #work directory where resources are stored
        )
execute_process(COMMAND ${RES_GEN_COMMAND})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

include_directories(
        ${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/res/build/include
        ${PROJECT_SOURCE_DIR}/libs/imgui
        ${PROJECT_SOURCE_DIR}/libs/tfd
        ${PROJECT_SOURCE_DIR}/libs/stb
        ${PROJECT_SOURCE_DIR}
)

file(GLOB all_SRCS
        ${PROJECT_SOURCE_DIR}/inc/*.h
        ${PROJECT_SOURCE_DIR}/inc/*.hpp
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.c
        ${PROJECT_SOURCE_DIR}/res/build/src/resources.cpp
        ${PROJECT_SOURCE_DIR}/glad/src/glad.c
        ${PROJECT_SOURCE_DIR}/libs/imgui/*.cpp
        ${PROJECT_SOURCE_DIR}/libs/tfd/tinyfiledialogs.c
        )

if( WIN32 )
    add_executable(${PROJECT_NAME} ${all_SRCS}
            ${PROJECT_SOURCE_DIR}/res/win-res.rc
            ${PROJECT_SOURCE_DIR}/res/build/src/win.rc)

    ## On Windows disable console if not in debug mode
    if( WIN32 AND NOT (CMAKE_BUILD_TYPE STREQUAL Debug) )
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static -mwindows")
        elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
            set_target_properties(${PROJECT_NAME} PROPERTIES
                    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
                    )
        endif()
    endif()
else()
    add_executable(${PROJECT_NAME} ${all_SRCS})
    # link object files created by resource builder
    file(GLOB all_OBJS ${PROJECT_SOURCE_DIR}/res/build/objs/*.o)
    target_link_libraries(${PROJECT_NAME} ${all_OBJS})
endif()

target_link_libraries(${PROJECT_NAME} glfw)
