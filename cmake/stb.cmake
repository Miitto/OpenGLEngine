add_subdirectory(../vendor/stb stb_cmake)

function(link_stb TARGET_NAME ACCESS)
  target_link_libraries(${TARGET_NAME} ${ACCESS} stb::stb)
endfunction()