# Async Server Example
add_executable(AsyncServer async_server.cpp)
target_link_libraries(AsyncServer PRIVATE LandingPad::Icelander)
target_include_directories(AsyncServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(AsyncServer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Async Client Example
add_executable(AsyncClient async_client.cpp)
target_link_libraries(AsyncClient PRIVATE LandingPad::Icelander)
target_include_directories(AsyncClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(AsyncClient PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(AsyncServer PRIVATE /W4)
    target_compile_options(AsyncClient PRIVATE /W4)
else()
    target_compile_options(AsyncServer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(AsyncClient PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Install networking examples
install(TARGETS AsyncServer AsyncClient
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples/networking
)
