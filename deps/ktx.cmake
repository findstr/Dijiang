set(ktx_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/texture.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/hashlist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/checkheader.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/swap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/memstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ktx/lib/filestream.c)
add_library(ktx STATIC ${ktx_sources})
target_include_directories(ktx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ktx)
