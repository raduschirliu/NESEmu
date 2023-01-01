# argh - https://github.com/adishavit/argh/releases/tag/v1.3.1
set(INCLUDES ${INCLUDES} argh-1.3.1/)

# GLEW - http://glew.sourceforge.net/
set(INCLUDES ${INCLUDES} glew-2.1.0/include)
set(SOURCES ${SOURCES} glew-2.1.0/src/glew.c)
# GLEW on windows gets angry if not built with this definition for a static lib...
set(DEFINITIONS ${DEFINITIONS} GLEW_STATIC)

# GLFW - https://www.glfw.org/
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw-3.3.2)
set(LIBRARIES ${LIBRARIES} glfw ${GLFW_LIBRARIES})
set(INCLUDES ${INCLUDES} glfw-3.3.2/include)
# Prevent GLFW from trying to load its own OpenGL headers
set(DEFINITIONS ${DEFINITIONS} GLFW_INCLUDE_NONE)

# vivid - https://github.com/gurki/vivid/releases/tag/v2.2.1
set(INCLUDES ${INCLUDES} vivid-2.2.1/include vivid-2.2.1/dependencies/glm)

# glm - https://glm.g-truc.net/0.9.9/index.html
set(DEFINITIONS ${DEFINITIONS} GLM_FORCE_SWIZZLE)
set(DEFINITIONS ${DEFINITIONS} GLM_ENABLE_EXPERIMENTAL)
set(INCLUDES ${INCLUDES} glm-0.9.9.7/)

# FMT - https://fmt.dev/latest/index.html
set(FMT_TES OFF CACHE BOOL "" FORCE)
add_subdirectory(fmt-7.0.3)
set(LIBRARIES ${LIBRARIES} fmt::fmt)
set(INCLUDES ${INCLUDES} fmt-7.0.3/include)

# OpenGL
find_package(OpenGL REQUIRED)
set(LIBRARIES ${LIBRARIES} ${OPENGL_gl_LIBRARY})

# STB
set(INCLUDES ${INCLUDES} stb-2.26)

# OS-dependent options
if(UNIX)
	set(LIBRARIES ${LIBRARIES} pthread GL dl)
endif(UNIX)
if (WIN32)
endif()

add_library(thirdparty STATIC)
target_sources(thirdparty PUBLIC ${SOURCES})
target_link_libraries(thirdparty PUBLIC ${LIBRARIES})
target_include_directories(thirdparty PUBLIC ${INCLUDES})
target_compile_definitions(thirdparty PUBLIC ${DEFINITIONS})

include(imgui.cmake)
