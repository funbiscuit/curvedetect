cmake_minimum_required(VERSION 3.13)
project(curvedetect)

set(CMAKE_CXX_STANDARD 11)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/glfw)

include_directories(
        ${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/libs/imgui
        ${PROJECT_SOURCE_DIR}/libs/tfd
        ${PROJECT_SOURCE_DIR}/libs/stb
        ${PROJECT_SOURCE_DIR}/libs/eigen
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

file(GLOB all_RES
        ${PROJECT_SOURCE_DIR}/res/*.o
        )

# works with both linux and mingw shell, but doesn't work with jetbrains clion since it's launching cmake from windows
# shell (as it seems to be doing).
# so if you're launching cmake on windows not from mingw shell you will need to rebuild resources manually
# by executing ./res/build-res.sh in mingw shell (work dir is project root)
set(RES_GEN_COMMAND bash -l -c "cd ${PROJECT_SOURCE_DIR} && bash ./res/build-res.sh" )

execute_process(COMMAND ${RES_GEN_COMMAND}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})


add_executable(${PROJECT_NAME} ${all_SRCS})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${all_RES})