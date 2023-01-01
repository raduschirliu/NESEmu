cmake_minimum_required(VERSION 3.19.0)
project(NESEmu)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#-------------------------------------------------------------------------------
# Compiler arguments
#-------------------------------------------------------------------------------

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	list(APPEND NESEMU_CXX_FLAGS ${NESEMU_CXX_FLAGS} "-Wall" "-pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	list(APPEND NESEMU_CXX_FLAGS ${NESEMU_CXX_FLAGS} "-fdiagnostics-color=always" "-Wall" "-pedantic" "-Wconversion")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_options("/permissive-")
	add_compile_options("/Zc:preprocessor")
	# Remove the default warning level.
	# https://stackoverflow.com/questions/58708772/cmake-project-in-visual-studio-gives-flag-override-warnings-command-line-warnin
	string(REGEX REPLACE "/W[3|4]" "" NESEMU_CXX_FLAGS "${NESEMU_CXX_FLAGS}")
	 # The above preprocessor warns about certain things in windows headers.
	 # Turn those off: https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5105?view=vs-2019
	add_compile_options("/wd5105")

	set(NESEMU_CXX_FLAGS ${NESEMU_CXX_FLAGS} /W3)
endif()

#-------------------------------------------------------------------------------
# Subdirectories
#-------------------------------------------------------------------------------

add_subdirectory(thirdparty)
add_subdirectory(nesemu)
