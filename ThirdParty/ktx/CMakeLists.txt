cmake_minimum_required(VERSION 3.10)
project(ktx)

# 设置C/C++标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# 手动列出所有源文件（比 GLOB 更可靠）
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

# 创建静态库（改为 SHARED 可生成动态库）
add_library(ktx STATIC ${LIB_SOURCES})

target_include_directories(ktx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/other_include

)
# 如果需要 OpenGL/Vulkan 链接
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
target_link_libraries(ktx PRIVATE OpenGL::GL Vulkan::Vulkan)