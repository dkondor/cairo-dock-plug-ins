
add_subdirectory(src)
add_subdirectory(data)

if (PYTHON3_FOUND)
	add_subdirectory(watcher)
endif()
