set(GLAD_SOURCE_FILES
    glad/src/gl.c
)

set(GLAD_HEADER_FILES
    glad/include/glad/gl.h
    glad/include/KHR/khrplatform.h
)

set(IMGUI_SOURCE_FILES
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/gui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
)

set(IMGUI_HEADER_FILES
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_opengl3_loader.h
    imgui/backends/imgui_impl_opengl3.h
    imgui/gui.hpp
    imgui/imconfig.h
    imgui/imgui_internal.h
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
)

target_sources(common 
    PUBLIC 
        ${GLAD_HEADER_FILES} ${IMGUI_HEADER_FILES}
    PRIVATE 
        ${GLAD_SOURCE_FILES} ${IMGUI_SOURCE_FILES}
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)