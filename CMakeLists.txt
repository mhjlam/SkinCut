# SkinCut - Real-Time Simulation and Visualization of Cutting Wounds
# CMake Build System for DirectX 11 Application
# Author: Maurits Lam
# Date: 2025
#
# Build Instructions:
#   From root directory:
#     cmake -S . -B Build
#     cmake --build Build --config Release
#   For Visual Studio:
#     cmake -S . -B Build -G "Visual Studio 17 2022" -A x64
#     cmake --build Build --config Release

cmake_minimum_required(VERSION 3.20)
project(SkinCut VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Windows-only project (DirectX 11 requirement)
if(NOT WIN32)
    message(FATAL_ERROR "SkinCut requires Windows for DirectX 11 support")
endif()

# Enable vcpkg manifest mode if vcpkg.json is present
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    # Enable manifest mode and point to the correct vcpkg.json location
    if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg.json")
        set(VCPKG_MANIFEST_DIR "${CMAKE_SOURCE_DIR}" CACHE STRING "Vcpkg manifest directory")
        set(VCPKG_MANIFEST_MODE ON CACHE BOOL "Enable vcpkg manifest mode")
        # For Visual Studio generator, automatically enable vcpkg manifest in MSBuild
        if(CMAKE_GENERATOR MATCHES "Visual Studio")
            set(CMAKE_VS_GLOBALS "VcpkgEnableManifest=true")
        endif()
    endif()
endif()

# Enable modern CMake policies
cmake_policy(SET CMP0074 NEW)  # find_package uses <PackageName>_ROOT variables

# Project directories
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/Bin)
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/Resources)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/Shaders)

# Set clean output directories - executables go to Bin/Debug or Bin/Release
set(BIN_OUTPUT_DIR ${BIN_DIR}/$<CONFIG>)
set(BUILD_INTERMEDIATE_DIR ${CMAKE_BINARY_DIR}/obj/$<CONFIG>)

# Create necessary directories
file(MAKE_DIRECTORY ${BIN_DIR})
file(MAKE_DIRECTORY ${BIN_DIR}/Debug)
file(MAKE_DIRECTORY ${BIN_DIR}/Release)

# Compiler-specific flags (MSVC only for DirectX 11)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc")
    # DirectX 11 and Windows-specific definitions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D WIN32 /D NOMINMAX /D WIN32_LEAN_AND_MEAN")
    # Disable specific MSVC warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4201")
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /D _DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /D NDEBUG")
else()
    message(WARNING "SkinCut is designed for MSVC compiler on Windows")
endif()

# Find packages using vcpkg
find_package(directxtex CONFIG REQUIRED)
find_package(directxtk CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Collect source files from the actual SkinCut project structure
file(GLOB SKINCUT_SOURCES
    ${SOURCE_DIR}/*.cpp
)

file(GLOB SKINCUT_HEADERS
    ${SOURCE_DIR}/*.hpp
)

# Print discovered files for debugging
message(STATUS "Discovered SkinCut source files:")
foreach(src_file ${SKINCUT_SOURCES})
    message(STATUS "  ${src_file}")
endforeach()

# Create Windows executable with only SkinCut sources (libraries come from vcpkg)
add_executable(SkinCut WIN32 ${SKINCUT_SOURCES} ${SKINCUT_HEADERS})

# Set clean output directories and Windows subsystem
set_target_properties(SkinCut PROPERTIES
    # Executable goes to Bin/Debug or Bin/Release
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}/Release
    
    # Intermediate build files go to build directory (CMake manages this)
    ARCHIVE_OUTPUT_DIRECTORY ${BUILD_INTERMEDIATE_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BUILD_INTERMEDIATE_DIR}
    

)

# Enable Unicode support for wide character APIs
target_compile_definitions(SkinCut PRIVATE 
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# Add include directories
target_include_directories(SkinCut PRIVATE 
    ${SOURCE_DIR}
)

# Link vcpkg libraries and DirectX 11/Windows libraries
target_link_libraries(SkinCut PRIVATE 
    # vcpkg packages
    Microsoft::DirectXTex
    Microsoft::DirectXTK
    imgui::imgui
    nlohmann_json::nlohmann_json
    
    # DirectX 11 and Windows libraries
    d3d11
    dxgi
    d3dcompiler
    dxguid
    winmm
    comctl32
    user32
    gdi32
    ole32
    oleaut32
    uuid
    kernel32
)

# Copy resource files to the binary output directory
add_custom_command(TARGET SkinCut POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${RESOURCES_DIR}/" "${BIN_OUTPUT_DIR}/Resources/"
    COMMENT "Copying resource files to ${BIN_OUTPUT_DIR}/Resources/"
)

# HLSL Shader Compilation
# Find FXC compiler (DirectX Shader Compiler)
find_program(FXC_EXECUTABLE
    NAMES fxc.exe
    PATHS
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x86" 
        "$ENV{WindowsSdkDir}/bin/x64"
        "$ENV{WindowsSdkDir}/bin/x86"
        "C:/Program Files (x86)/Windows Kits/10/bin/*/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/*/x86"
    DOC "DirectX Shader Compiler (fxc.exe)"
)

if(NOT FXC_EXECUTABLE)
    message(FATAL_ERROR "DirectX Shader Compiler (fxc.exe) not found. Please install Windows SDK.")
endif()

message(STATUS "Found DirectX Shader Compiler: ${FXC_EXECUTABLE}")

# Create shader output directories at build time (not configure time)
# The directories will be created by the custom commands themselves

# Collect shader files
file(GLOB VERTEX_SHADERS "${SHADERS_DIR}/*.vs.hlsl")
file(GLOB PIXEL_SHADERS "${SHADERS_DIR}/*.ps.hlsl")
file(GLOB HEADER_SHADERS "${SHADERS_DIR}/*.h.hlsl")

# Shader compilation - silent mode
set(SHADER_COMMANDS "")

# Build all shader compilation commands
foreach(HEADER_SHADER ${HEADER_SHADERS})
    get_filename_component(HEADER_NAME ${HEADER_SHADER} NAME)
    list(APPEND SHADER_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HEADER_SHADER}" "${BIN_OUTPUT_DIR}/Resources/Shaders/${HEADER_NAME}"
    )
endforeach()

foreach(VERTEX_SHADER ${VERTEX_SHADERS})
    get_filename_component(SHADER_NAME ${VERTEX_SHADER} NAME_WE)
    set(OUTPUT_FILE "${BIN_OUTPUT_DIR}/Resources/Shaders/${SHADER_NAME}.vs.cso")
    list(APPEND SHADER_COMMANDS
        COMMAND ${FXC_EXECUTABLE} /nologo /T vs_4_0 /E main /Fo "${OUTPUT_FILE}" "${VERTEX_SHADER}"
    )
endforeach()

foreach(PIXEL_SHADER ${PIXEL_SHADERS})
    get_filename_component(SHADER_NAME ${PIXEL_SHADER} NAME_WE)
    set(OUTPUT_FILE "${BIN_OUTPUT_DIR}/Resources/Shaders/${SHADER_NAME}.ps.cso")
    list(APPEND SHADER_COMMANDS
        COMMAND ${FXC_EXECUTABLE} /nologo /T ps_4_0 /E main /Fo "${OUTPUT_FILE}" "${PIXEL_SHADER}"
    )
endforeach()

# Single command with all shader operations
add_custom_command(TARGET SkinCut POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_OUTPUT_DIR}/Resources/Shaders"
    ${SHADER_COMMANDS}
    COMMENT "Compiling HLSL shaders..."
)
# This would require fxc.exe or dxc.exe to be available
# For now, copy them as source files and let runtime compilation handle them

# Print build information
message(STATUS "SkinCut v${PROJECT_VERSION} Build Configuration:")
message(STATUS "  Platform:       ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:   C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Source Dir:     ${SOURCE_DIR}")
message(STATUS "  Binary Dir:     ${BIN_DIR}") 
message(STATUS "  Build Dir:      ${CMAKE_BINARY_DIR}")
message(STATUS "  Resources Dir:  ${RESOURCES_DIR}")
message(STATUS "  Build Flags:    ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

# Optional: Create install targets
install(TARGETS SkinCut
    RUNTIME DESTINATION Bin
    COMPONENT runtime
)

install(DIRECTORY ${RESOURCES_DIR}/
    DESTINATION Bin/Resources
    COMPONENT resources
)

# Package configuration
set(CPACK_PACKAGE_NAME "SkinCut")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Real-Time Simulation and Visualization of Cutting Wounds")
set(CPACK_PACKAGE_VENDOR "Maurits Lam")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
include(CPack)
