# --- Your existing registration program ---
add_executable(RegisterCircles main.cxx)
target_link_libraries(RegisterCircles PRIVATE ${ITK_LIBRARIES})

# --- Add the new image generator program ---
add_executable(GenerateImages GenerateImages.cxx)
target_link_libraries(GenerateImages PRIVATE ${ITK_LIBRARIES})