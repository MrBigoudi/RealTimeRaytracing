target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/gl.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/)

# Add ImGui source files
file(GLOB IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/*.cpp)
# Include ImGui headers
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
# Add ImGui sources to the target
target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})