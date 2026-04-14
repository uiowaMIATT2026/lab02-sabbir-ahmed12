# Create executable
add_executable(RegisterCircles main.cpp)

# Link ITK libraries
target_link_libraries(RegisterCircles PRIVATE ${ITK_LIBRARIES})