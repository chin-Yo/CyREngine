cmake_minimum_required(VERSION 3.10)
project(CyREngine LANGUAGES CXX VERSION 0.1.0)
# ------ Options ----

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

include(CMakeDependentOption)

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Paths ----
set(CYRENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_PREFIX "${CYRENGINE_ROOT_DIR}/bin")
set(BINARY_ROOT_DIR "${CMAKE_INSTALL_PREFIX}/")


# 设置全局构建输出目录
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# 定义函数：设置每个配置的输出路径
function(set_output_directories target_type output_dir)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} config_upper)
        set(CMAKE_${target_type}_OUTPUT_DIRECTORY_${config_upper}
            ${PROJECT_BINARY_DIR}/${output_dir}/${config}
            CACHE INTERNAL ""
        )
    endforeach()
endfunction()

# 设置可执行文件、库的输出路径
set_output_directories(RUNTIME bin)
set_output_directories(LIBRARY lib)
set_output_directories(ARCHIVE lib)

# ---- Global properties ----
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# ---- Subdirectories ----
add_subdirectory(ThirdParty)
add_subdirectory(Engine/Source)

# ---- Global path ----
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/ThirdParty" CACHE PATH "第三方库根目录")
set(CMAKE_DIR "${CMAKE_SOURCE_DIR}/CMake" CACHE PATH ".cmake目录")


