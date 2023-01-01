# TODO: Stop using globs
file(GLOB_RECURSE SOURCES
    src/*
)
set(INCLUDES ${INCLUDES} src)

# Get transient linker dependencies for thirdparty libs
set(THIRDPARTY_LINK_DEPS "")
get_target_property(THIRDPARTY_LINK_DEPS thirdparty INTERFACE_LINK_LIBRARIES)

# Create target
add_executable(nesemu ${SOURCES})

target_include_directories(nesemu PRIVATE src)
target_link_libraries(nesemu PUBLIC thirdparty ${THIRDPARTY_LINK_DEPS})
target_compile_definitions(nesemu PRIVATE ${NESEMU_CXX_DEFINITIONS})
target_compile_options(nesemu PRIVATE ${NESEMU_CXX_FLAGS})
set_target_properties(nesemu PROPERTIES INSTALL_RPATH "./" BUILD_RPATH "./")

# Copy shaders to build dir
add_custom_command(
	TARGET nesemu POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_SOURCE_DIR}/shaders
			$<TARGET_FILE_DIR:nesemu>/shaders
)

# Copy resources to build dir
add_custom_command(
	TARGET nesemu POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_SOURCE_DIR}/resources
			$<TARGET_FILE_DIR:nesemu>/resources
)
