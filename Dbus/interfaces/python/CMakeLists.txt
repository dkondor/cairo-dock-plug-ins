
if (NOT "${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	# copy *.py to current binary dir (cleaner)
	execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" ${CMAKE_CURRENT_BINARY_DIR})
	execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/CDApplet.py" ${CMAKE_CURRENT_BINARY_DIR})
	execute_process(COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/CairoDock.py" ${CMAKE_CURRENT_BINARY_DIR})
endif()

###########  byte-compile the python scripts to .pyc ###############
if (PYTHON2_FOUND)
	execute_process(COMMAND "${PYTHON2_EXECUTABLE}" "setup.py" "-q" "build" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()
if (PYTHON3_FOUND)
	execute_process(COMMAND "${PYTHON3_EXECUTABLE}" "setup.py" "-q" "build" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")  # if both are present, the build sub-folder will not have the same name, so we can build now and then install during the 'make install', which allows us to not build as root.
endif()

########### install files ###############
# Run a custom script with 'make install' 
add_custom_target(python_pyc ALL)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/PythonInstall.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake @ONLY)
set_target_properties(python_pyc PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/PythonInstall.cmake)
