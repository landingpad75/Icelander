# Tests CMakeLists.txt

# Unit Tests
add_executable(UnitTests unit_tests.cpp)
target_link_libraries(UnitTests PRIVATE LandingPad::Icelander)
target_include_directories(UnitTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(UnitTests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(UnitTests PRIVATE /W4)
else()
    target_compile_options(UnitTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable testing with CTest
enable_testing()

# Add test targets
add_test(NAME IcelanderUnitTests COMMAND UnitTests)

# Set test properties
set_tests_properties(IcelanderUnitTests PROPERTIES
    TIMEOUT 30
    PASS_REGULAR_EXPRESSION "All tests completed successfully!"
)

# Optional: Install test executable
install(TARGETS UnitTests
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)
