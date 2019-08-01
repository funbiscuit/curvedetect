cmake_minimum_required(VERSION 3.13)
project(curvedetect)

set(CMAKE_CXX_STANDARD 11)

# on linux compile resources
if( NOT WIN32 )
    set(RES_GEN_COMMAND bash -l -c "cd ${PROJECT_SOURCE_DIR}/res && bash ./build-res.sh" )

    execute_process(COMMAND ${RES_GEN_COMMAND}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

include_directories(
        ${PROJECT_SOURCE_DIR}/inc
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
        ${PROJECT_SOURCE_DIR}/glad/src/glad.c
        ${PROJECT_SOURCE_DIR}/libs/imgui/*.cpp
        ${PROJECT_SOURCE_DIR}/libs/tfd/tinyfiledialogs.c
        )

if( WIN32 )
    add_executable(${PROJECT_NAME} ${all_SRCS} ${PROJECT_SOURCE_DIR}/res/win-res.rc)

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
    file(GLOB all_RES
            ${PROJECT_SOURCE_DIR}/res/gen/*.o
            )
    add_executable(${PROJECT_NAME} ${all_SRCS})
    target_link_libraries(${PROJECT_NAME} ${all_RES})
endif()

target_link_libraries(${PROJECT_NAME} glfw)
