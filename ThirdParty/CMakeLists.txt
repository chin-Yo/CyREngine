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
    option(GLM_BUILD_TESTS "" OFF)              # Disable test build
    option(GLM_BUILD_LIBRARY "" OFF)            # Disable library build（header-only）
    option(GLM_ENABLE_CXX_17 "" ON)             # Enable C++20 support
    option(GLM_ENABLE_SIMD_AVX2 "" ON)          # Enable AVX2 instruction set
    option(GLM_ENABLE_FAST_MATH "" ON)          # Enable fast math

    add_subdirectory(glm)
    set_target_properties(glm PROPERTIES FOLDER ${third_party_folder}/glm)
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


# Check if the Vulkan SDK is installed
if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK at: $ENV{VULKAN_SDK}")
    
    # Define the interface library
    add_library(vulkan_sdk INTERFACE)
    
    # Set include directories
    target_include_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/Include")
    
    if(WIN32)
        target_link_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/Lib")
    else()
        target_link_directories(vulkan_sdk INTERFACE "$ENV{VULKAN_SDK}/lib")
    endif()
    
    target_link_libraries(vulkan_sdk INTERFACE vulkan-1)
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()

if(NOT TARGET SPIRV-Cross)
    # 只启用反射功能，关闭其他所有选项
    set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)  # 强制覆盖
    set(SPIRV_CROSS_ENABLE_REFLECT ON CACHE BOOL "" FORCE)
    option(SPIRV_CROSS_ENABLE_HLSL "Enable HLSL target support." OFF)
    option(SPIRV_CROSS_ENABLE_MSL "Enable MSL target support." OFF)
    option(SPIRV_CROSS_ENABLE_CPP "Enable C++ target support." ON)
    option(SPIRV_CROSS_ENABLE_C_API "Enable C API wrapper support in static library." OFF)
    option(SPIRV_CROSS_ENABLE_UTIL "Enable util module support." OFF)

# 确保静态库模式（因为反射可能需要它）
    option(SPIRV_CROSS_STATIC "Build the C and C++ API as static libraries." ON)
    option(SPIRV_CROSS_SHARED "Build the C API as a single shared library." OFF)
    option(SPIRV_CROSS_CLI "Build the CLI binary. Requires SPIRV_CROSS_STATIC." OFF)

# 其他选项默认关闭
    option(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS "Instead of throwing exceptions assert" OFF)
    option(SPIRV_CROSS_ENABLE_TESTS "Enable SPIRV-Cross tests." OFF)
    option(SPIRV_CROSS_SANITIZE_ADDRESS "Sanitize address" OFF)
    option(SPIRV_CROSS_SANITIZE_MEMORY "Sanitize memory" OFF)
    option(SPIRV_CROSS_SANITIZE_THREADS "Sanitize threads" OFF)
    option(SPIRV_CROSS_SANITIZE_UNDEFINED "Sanitize undefined" OFF)
    option(SPIRV_CROSS_FORCE_STL_TYPES "Force use of STL types." OFF)
    option(SPIRV_CROSS_SKIP_INSTALL "Skips installation targets." OFF)
    option(SPIRV_CROSS_WERROR "Fail build on warnings." OFF)
    option(SPIRV_CROSS_MISC_WARNINGS "Misc warnings useful for Travis runs." OFF)
    option(SPIRV_CROSS_FORCE_PIC "Force position-independent code for all targets." OFF)

# 添加 SPIRV-Cross 子目录
    add_subdirectory(SPIRV-Cross)
    #set_target_properties(SPIRV-Cross PROPERTIES FOLDER ${third_party_folder}/SPIRV-Cross)
endif()

if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "Build SPDLOG examples" OFF)
    option(SPDLOG_INSTALL "Install SPDLOG" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_folder}/spdlog)
endif()
# # 自动递归搜索所有头文件
# file(GLOB_RECURSE HEADER_FILES 
#     "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann-json/*.h"
#     "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann-json/*.hpp"
# )

# set(PROJECT_NAME nlohmann-json)

# # 创建INTERFACE库
# add_library(${PROJECT_NAME} INTERFACE)

# # 添加头文件
# target_sources(${PROJECT_NAME} INTERFACE
#     ${HEADER_FILES}
# )

# set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "nlohmann-json")
