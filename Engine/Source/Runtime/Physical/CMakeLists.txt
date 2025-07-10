

set(TARGET_NAME PhysicsEngine)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 动态搜索源文件
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "Source/*.cpp"
    "Source/*.cxx"
)

# 获取头文件(仅用于IDE分组)
file(GLOB_RECURSE LIB_HEADERS
    "Include/*.h"
    "Include/*.hpp"
)

# 创建静态库
add_library(${TARGET_NAME} STATIC ${LIB_SOURCES})

# 设置包含目录
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# 为IDE组织文件夹结构
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "Source" FILES ${LIB_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Include PREFIX "Include" FILES ${LIB_HEADERS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Runtime/Physical")
