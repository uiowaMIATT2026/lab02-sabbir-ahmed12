cmake_minimum_required(VERSION 3.16)

project(RegisterCircles)

# Find ITK
find_package(ITK REQUIRED COMPONENTS
    ITKCommon
    ITKIOImageBase
    ITKIOMeta
)

# Add subdirectory
add_subdirectory(src)