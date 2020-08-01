add_subdirectory(portable-file-dialogs)

# create library for stb which doesn't have cmake file
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)
add_library(STB::STB ALIAS stb)
