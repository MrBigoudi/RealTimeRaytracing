file(GLOB SOURCE_FILES "*.cpp")

target_sources(${PROJECT_NAME} PUBLIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)

add_subdirectory(shaders)