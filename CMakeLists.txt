cmake_minimum_required(VERSION 3.15)
project(GameEngineWorkspace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Configure output directories similar to premake (bin and bin-int)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int/$<CONFIG>)

# ---------------------------------------------------------------------
# Project: GameEngine (Shared Library / DLL)
# ---------------------------------------------------------------------
file(GLOB_RECURSE GAMEENGINE_SOURCES "GameEngine/src/*.cpp" "GameEngine/src/*.h")

add_library(GameEngine SHARED ${GAMEENGINE_SOURCES})

# Include directories
target_include_directories(GameEngine PUBLIC 
    GameEngine/vendor/spdlog/include
    GameEngine/src
)

# Compiler definitions for Windows
if(WIN32)
    target_compile_definitions(GameEngine PUBLIC GE_PLATFORM_WINDOWS)
    target_compile_definitions(GameEngine PRIVATE GE_BUILD_DLL)
endif()

# Configuration-specific defines
target_compile_definitions(GameEngine PRIVATE
    $<$<CONFIG:Debug>:GE_DEBUG>
    $<$<CONFIG:Release>:GE_RELEASE>
    $<$<CONFIG:Dist>:GE_DIST>
)

# ---------------------------------------------------------------------
# Project: Sandbox (Console Application)
# ---------------------------------------------------------------------
file(GLOB_RECURSE SANDBOX_SOURCES "Sandbox/src/*.cpp" "Sandbox/src/*.h")

add_executable(Sandbox ${SANDBOX_SOURCES})

# Include directories
target_include_directories(Sandbox PUBLIC 
    GameEngine/vendor/spdlog/include
    GameEngine/src
)

# Link GameEngine to Sandbox
target_link_libraries(Sandbox PRIVATE GameEngine)

# Compiler definitions for Windows
if(WIN32)
    target_compile_definitions(Sandbox PUBLIC GE_PLATFORM_WINDOWS)
endif()

# Configuration-specific defines
target_compile_definitions(Sandbox PRIVATE
    $<$<CONFIG:Debug>:GE_DEBUG>
    $<$<CONFIG:Release>:GE_RELEASE>
    $<$<CONFIG:Dist>:GE_DIST>
)