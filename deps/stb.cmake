file(GLOB stb_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/stb/*.cpp")
add_library(stb STATIC ${stb_sources})
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)
