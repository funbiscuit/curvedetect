cmake_minimum_required(VERSION 3.13)
project(curvedetect LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

# add dependencies
add_subdirectory(libs)

# add resources library
add_subdirectory(res)

# add gui executable
add_subdirectory(app-gui)
