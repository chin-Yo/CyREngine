#Creates a folder called "ThirdParty" to organize all third-party libraries
set(third_party_folder "ThirdParty")
#Forces all libraries to be built as static libraries (not shared/DLLs)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

#source_group("FilterName" FILES file1 file2 ...)

if(NOT TARGET glfw) # glfw-3.4
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
endif()

if(NOT TARGET glm::glm)  # GLM 1.0.1
    # GLM 配置选项（覆盖默认设置）
    option(GLM_BUILD_TESTS "" OFF)               # 禁用测试构建
    option(GLM_BUILD_LIBRARY "" OFF)            # 禁用库构建（header-only）
    option(GLM_ENABLE_CXX_17 "" ON)             # 启用C++20支持
    option(GLM_ENABLE_SIMD_AVX2 "" ON)          # 启用AVX2指令集
    option(GLM_ENABLE_FAST_MATH "" ON)          # 启用快速数学
    # 包含GLM子目录
    add_subdirectory(glm)
    # 可选：将GLM目标归类到第三方库文件夹
    set_target_properties(glm PROPERTIES FOLDER ${third_party_folder}/glm)
    # 可选：打印配置状态
    message(STATUS "GLM configured: C++20=${GLM_ENABLE_CXX_20}, AVX2=${GLM_ENABLE_SIMD_AVX2}")
endif()

if(NOT TARGET imgui)
    add_subdirectory(imgui)
    set_target_properties(imgui PROPERTIES FOLDER ${third_party_folder}/imgui)
endif()

if(NOT TARGET ktx)
    add_subdirectory(ktx)
    set_target_properties(ktx PROPERTIES FOLDER ${third_party_folder}/ktx)
endif()

# 获取该目录下的所有 .c 和 .h 文件
file(GLOB TINYGLTF_HEADERS "${THIRD_PARTY_DIR}/tinygltf/*.h" "${THIRD_PARTY_DIR}/tinygltf/*.hpp")
# 把这些文件加入名为 "ThirdParty/ktx" 的筛选器中
source_group("ThirdParty\\tinygltf" FILES ${TINYGLTF_HEADERS})


# 检查环境变量
if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK at: $ENV{VULKAN_SDK}")
    
    # 定义一个仅供主项目使用的目标
    add_library(vulkan_sdk INTERFACE)
    
    # 设置头文件路径
    target_include_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/Include")
    
    # 设置库路径（跨平台）
    if(WIN32)
        target_link_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/Lib")
    else()
        target_link_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/lib")
    endif()
    
    # 链接 vulkan-1 库
    target_link_libraries(vulkan_sdk INTERFACE vulkan-1)
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()