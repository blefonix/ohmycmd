cmake_minimum_required(VERSION 3.19)

project(ohmycmd VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/open.mp-sdk)

add_library(ohmycmd MODULE
    src/main.cpp
)

target_link_libraries(ohmycmd PRIVATE OMP-SDK)

target_compile_definitions(ohmycmd PRIVATE
    OHMYCMD_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    OHMYCMD_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    OHMYCMD_VERSION_PATCH=${PROJECT_VERSION_PATCH}
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
