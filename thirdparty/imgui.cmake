set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

target_compile_definitions(thirdparty PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

target_sources(thirdparty
    PUBLIC
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(thirdparty
    PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)
