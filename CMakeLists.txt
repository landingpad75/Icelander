cmake_minimum_required(VERSION 3.20)
project(Icelander VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(outside/enet)

add_library(Icelander STATIC)
add_library(LandingPad::Icelander ALIAS Icelander)

file(GLOB_RECURSE ICELANDER_SOURCES
    "src/library.cpp"
    "src/endpoint.cpp"
    "src/packet.cpp"
    "src/peer.cpp"
    "src/event_dispatcher.cpp"
    "src/host.cpp"
    "src/async.cpp"
)

file(GLOB_RECURSE ICELANDER_HEADERS
    "include/*.hpp"
    "include/*.h"
)

target_sources(Icelander PRIVATE ${ICELANDER_SOURCES})

target_include_directories(Icelander
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/outside/enet/include
)

target_link_libraries(Icelander
    PUBLIC enet
)

if(WIN32)
    target_link_libraries(Icelander PRIVATE ws2_32 winmm)
endif()

set_target_properties(Icelander PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "${ICELANDER_HEADERS}"
)

if(MSVC)
    target_compile_options(Icelander PRIVATE /W4)
else()
    target_compile_options(Icelander PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(GNUInstallDirs)

# Add examples subdirectory
add_subdirectory(examples)

install(TARGETS Icelander enet
    EXPORT IcelanderTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Icelander
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT IcelanderTargets
    FILE IcelanderTargets.cmake
    NAMESPACE LandingPad::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Icelander
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/IcelanderConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/IcelanderConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Icelander
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/IcelanderConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/IcelanderConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/IcelanderConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Icelander
)

export(EXPORT IcelanderTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/IcelanderTargets.cmake"
    NAMESPACE LandingPad::
)

export(PACKAGE Icelander)
