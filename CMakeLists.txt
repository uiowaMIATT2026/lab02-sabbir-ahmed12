# Require a minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Define the project name and optionally its version/language
project(RegisterCircles VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_subdirectory(src)