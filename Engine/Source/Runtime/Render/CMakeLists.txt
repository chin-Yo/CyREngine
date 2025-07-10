
set(TARGET_NAME Render)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 动态搜索源文件
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "Source/*.cpp"
    "Source/*.cxx"
    "*.cpp"
)

# 获取头文件(仅用于IDE分组)
file(GLOB_RECURSE LIB_HEADERS
    "Include/*.h"
    "Include/*.hpp"
)

# 创建exe
add_executable(${TARGET_NAME} WIN32 ${LIB_SOURCES})

# 设置包含目录
target_include_directories(${TARGET_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/Core
    ${RUNTIME_DIR}/Physical/Include
    ${RUNTIME_DIR}/Physical/Include
    ${THIRD_PARTY_DIR}/stb
    ${THIRD_PARTY_DIR}/ktx/include
    ${THIRD_PARTY_DIR}/tinygltf
    ${THIRD_PARTY_DIR}/imgui
)

# 检查环境变量
if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK at: $ENV{VULKAN_SDK}")
    # 设置头文件路径
    target_include_directories(${TARGET_NAME} PUBLIC "$ENV{VULKAN_SDK}/include")
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()

target_include_directories(${TARGET_NAME} PUBLIC 
    ${THIRD_PARTY_DIR}/glm
)

target_link_libraries(${TARGET_NAME} PUBLIC PhysicsEngine)
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PUBLIC glm)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC ktx)
target_link_libraries(${TARGET_NAME} PUBLIC vulkan_sdk)


# 为IDE组织文件夹结构
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "Source" FILES ${LIB_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Include PREFIX "Include" FILES ${LIB_HEADERS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Runtime/Physical")
