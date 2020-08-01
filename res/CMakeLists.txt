#generate files for resource compilation
find_package( PythonInterp 3.0 REQUIRED )
execute_process(COMMAND ${PYTHON_EXECUTABLE}
        ${PROJECT_SOURCE_DIR}/libs/resource-builder/resource-builder.py #path to python script that builds resources
        ${CMAKE_CURRENT_SOURCE_DIR}                                     #work directory where resources are stored
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# link to generated resources
add_subdirectory(build)

# On Windows add .rc file with app icon to resources library
# CurvedetectRes is the name of auto generated library (declared in resources.json)
if( WIN32 )
    target_sources(CurvedetectRes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/win-res.rc)
endif()
