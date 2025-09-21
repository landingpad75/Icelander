# Comprehensive Example
add_executable(ComprehensiveExample comprehensive_example.cpp)
target_link_libraries(ComprehensiveExample PRIVATE LandingPad::Icelander)
target_include_directories(ComprehensiveExample PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(ComprehensiveExample PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Performance Benchmark
add_executable(PerformanceBenchmark performance_benchmark.cpp)
target_link_libraries(PerformanceBenchmark PRIVATE LandingPad::Icelander)
target_include_directories(PerformanceBenchmark PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../outside/enet/include
)

set_target_properties(PerformanceBenchmark PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(ComprehensiveExample PRIVATE /W4)
    target_compile_options(PerformanceBenchmark PRIVATE /W4)
else()
    target_compile_options(ComprehensiveExample PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(PerformanceBenchmark PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional: Install advanced examples
install(TARGETS ComprehensiveExample PerformanceBenchmark
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples/advanced
)
