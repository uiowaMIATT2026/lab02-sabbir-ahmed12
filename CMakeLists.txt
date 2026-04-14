cmake_minimum_required(VERSION 3.16)

project(RegisterCircles)

# Find ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Add subdirectory
add_subdirectory(src)