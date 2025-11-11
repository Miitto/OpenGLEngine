add_subdirectory(../vendor/tinygltf tinygltf_cmake)

function(link_tinygltf TARGET_NAME ACCESS)
  target_link_libraries(${TARGET_NAME} ${ACCESS} tinygltf::tinygltf)
endfunction()