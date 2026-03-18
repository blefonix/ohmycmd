cmake_minimum_required(VERSION 3.19)

project(ohmycmd VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_subdirectory(external/open.mp-sdk)

add_library(ohmycmd_core STATIC
    src/command_registry.cpp
    src/command_parser.cpp
)

target_include_directories(ohmycmd_core PUBLIC
    ${PROJECT_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ohmycmd_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(ohmycmd MODULE
    src/main.cpp
    src/command_registry.cpp
    src/command_parser.cpp
)

target_link_libraries(ohmycmd PRIVATE OMP-SDK)

target_include_directories(ohmycmd PRIVATE
    ${PROJECT_SOURCE_DIR}/external/pawn-amx
    ${PROJECT_SOURCE_DIR}/external/pawn-amx/amx
)

target_compile_definitions(ohmycmd PRIVATE
    OHMYCMD_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    OHMYCMD_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    OHMYCMD_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    HAVE_STDINT_H=1
)

# Current open.mp runtime in Blefonix is 32-bit; keep component ABI compatible by default.
if(UNIX AND NOT APPLE)
    target_compile_options(ohmycmd PRIVATE -m32)
    target_link_options(ohmycmd PRIVATE -m32)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ohmycmd PRIVATE -Wno-attributes)
endif()

# open.mp loads component files by filename in the components directory.
# Remove the default 'lib' prefix so the output is ohmycmd.so.
set_target_properties(ohmycmd PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ohmycmd"
)

option(OHMYCMD_BUILD_TESTING "Build test and benchmark binaries" ON)

if(BUILD_TESTING AND OHMYCMD_BUILD_TESTING)
    add_executable(ohmycmd_test_parser tests/test_parser.cpp)
    target_link_libraries(ohmycmd_test_parser PRIVATE ohmycmd_core)
    add_test(NAME ohmycmd_test_parser COMMAND ohmycmd_test_parser)

    add_executable(ohmycmd_test_registry tests/test_registry.cpp)
    target_link_libraries(ohmycmd_test_registry PRIVATE ohmycmd_core)
    add_test(NAME ohmycmd_test_registry COMMAND ohmycmd_test_registry)

    add_executable(ohmycmd_fuzz_parser tests/fuzz_parser.cpp)
    target_link_libraries(ohmycmd_fuzz_parser PRIVATE ohmycmd_core)
    add_test(NAME ohmycmd_fuzz_parser COMMAND ohmycmd_fuzz_parser 150000)

    add_executable(ohmycmd_stress_registry tests/stress_registry.cpp)
    target_link_libraries(ohmycmd_stress_registry PRIVATE ohmycmd_core)
    add_test(NAME ohmycmd_stress_registry COMMAND ohmycmd_stress_registry 12000 220000)

    add_executable(ohmycmd_bench_dispatch benchmarks/dispatch_benchmark.cpp)
    target_link_libraries(ohmycmd_bench_dispatch PRIVATE ohmycmd_core)
endif()
