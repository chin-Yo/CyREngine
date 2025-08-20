cmake_minimum_required(VERSION 3.10)
project(ktx)

set(CMAKE_CXX_STANDARD 17)

set(LIB_SOURCES
    lib/checkheader.c
    lib/filestream.c
    lib/hashlist.c
    lib/memstream.c
    lib/swap.c
    lib/texture.c
    lib/vkloader.c
)

add_definitions(-DKTX_OPENGL=1)

add_library(ktx STATIC ${LIB_SOURCES})

target_include_directories(ktx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/other_include

)
