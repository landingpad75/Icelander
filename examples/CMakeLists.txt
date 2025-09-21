# Add all example subdirectories
add_subdirectory(basic)
add_subdirectory(networking)
add_subdirectory(advanced)
add_subdirectory(tests)

# Create a convenience target to build all examples
add_custom_target(examples
    DEPENDS
        PacketBuilderExample
        SyncExample
        AsyncServer
        AsyncClient
        ComprehensiveExample
        PerformanceBenchmark
        UnitTests
    COMMENT "Building all Icelander examples"
)

# Create individual category targets
add_custom_target(basic_examples
    DEPENDS
        PacketBuilderExample
        SyncExample
    COMMENT "Building basic examples"
)

add_custom_target(networking_examples
    DEPENDS
        AsyncServer
        AsyncClient
    COMMENT "Building networking examples"
)

add_custom_target(advanced_examples
    DEPENDS
        ComprehensiveExample
        PerformanceBenchmark
    COMMENT "Building advanced examples"
)

add_custom_target(tests
    DEPENDS
        UnitTests
    COMMENT "Building tests"
)

# Print summary
message(STATUS "Configured Icelander examples:")
message(STATUS "  Basic examples: PacketBuilderExample, SyncExample")
message(STATUS "  Networking examples: AsyncServer, AsyncClient")
message(STATUS "  Advanced examples: ComprehensiveExample, PerformanceBenchmark")
message(STATUS "  Tests: UnitTests")
message(STATUS "Build targets available:")
message(STATUS "  make examples          - Build all examples")
message(STATUS "  make basic_examples    - Build basic examples only")
message(STATUS "  make networking_examples - Build networking examples only")
message(STATUS "  make advanced_examples - Build advanced examples only")
message(STATUS "  make tests             - Build tests only")
