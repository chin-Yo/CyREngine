# 设置项目名称
project(imgui)

# 查找当前目录下所有的cpp文件
file(GLOB LIB_SOURCES "*.cpp")

# 添加库目标
add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

# 包含当前目录为头文件搜索路径
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 可选：设置C++标准
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)