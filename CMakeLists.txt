# CMakeLists.txt for miniGit
#
# Defines the project, sets the C++ standard, and configures the build.

cmake_minimum_required(VERSION 3.16)

# Project name and version
project(miniGit VERSION 1.0)

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(MINIGIT_SOURCES
    src/main.cpp
    src/repository.cpp
    src/objects.cpp
    src/index.cpp
    src/utils.cpp
    src/commands.cpp
    src/branch.cpp
    src/merkle.cpp
)

# Add an executable target
# The main entry point for our command-line tool
add_executable(miniGit ${MINIGIT_SOURCES})

# Link OpenSSL for SHA-1 hashing
find_package(OpenSSL REQUIRED)
target_link_libraries(miniGit PRIVATE OpenSSL::Crypto)

# Include directory for header files
target_include_directories(miniGit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Enable testing with CTest
enable_testing()

# Add tests in the future
# add_subdirectory(tests)
