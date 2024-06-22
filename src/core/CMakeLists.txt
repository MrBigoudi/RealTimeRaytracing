set(CORE_SOURCE_FILES
    errorHandler.cpp
    input.cpp
)

set(CORE_HEADER_FILES
    errorHandler.hpp
    input.hpp
)

target_sources(common 
    PUBLIC 
        ${CORE_HEADER_FILES}
    PRIVATE
        ${CORE_SOURCE_FILES}
)

target_include_directories(common 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
)