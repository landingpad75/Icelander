# Packet Builder Example
add_executable(PacketBuilderExample packet_builder_example.cpp)
target_link_libraries(PacketBuilderExample PRIVATE LandingPad::Icelander)
target_include_directories(PacketBuilderExample PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(PacketBuilderExample PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Simple Synchronous Example
add_executable(SyncExample sync_example.cpp)
target_link_libraries(SyncExample PRIVATE LandingPad::Icelander)
target_include_directories(SyncExample PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(SyncExample PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(PacketBuilderExample PRIVATE /W4)
    target_compile_options(SyncExample PRIVATE /W4)
else()
    target_compile_options(PacketBuilderExample PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(SyncExample PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Install basic examples
install(TARGETS PacketBuilderExample SyncExample
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples/basic
)
