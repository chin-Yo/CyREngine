cmake_policy(SET CMP0079 NEW)  # 允许从外部目录链接目标

set(third_party_folder "ThirdParty")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


# vulkan
add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h)
target_include_directories(vulkan SYSTEM INTERFACE ${VULKAN_INCLUDE_DIR})

target_compile_definitions(vulkan INTERFACE VK_NO_PROTOTYPES)

if(VKB_ENABLE_PORTABILITY)
    # When portability is enabled, must enable Vulkan beta extensions for access to VK_KHR_portability_subset
    target_compile_definitions(vulkan INTERFACE VK_ENABLE_BETA_EXTENSIONS)
endif()

if(ANDROID)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_ANDROID_KHR)
elseif(WIN32)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    # Choose WSI based on VKB_WSI_SELECTION
    if (VKB_WSI_SELECTION STREQUAL XCB OR VKB_WSI_SELECTION STREQUAL XLIB OR VKB_WSI_SELECTION STREQUAL WAYLAND)
        find_package(PkgConfig REQUIRED)
    endif()
    if (VKB_WSI_SELECTION STREQUAL XCB)
        pkg_check_modules(XCB xcb REQUIRED)
        if (XCB_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL XLIB)
        pkg_check_modules(X11 x11 REQUIRED)
        if (X11_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_XLIB_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
        pkg_check_modules(WAYLAND wayland-client REQUIRED)
        if (WAYLAND_FOUND)
            target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_WAYLAND_KHR)
        endif()
    elseif (VKB_WSI_SELECTION STREQUAL D2D)
        set(DIRECT_TO_DISPLAY TRUE)
        set(DIRECT_TO_DISPLAY TRUE PARENT_SCOPE)
        target_compile_definitions(vulkan INTERFACE VK_USE_PLATFORM_DISPLAY_KHR)
    else()
        message(FATAL_ERROR "Unknown WSI")
    endif()
endif()

# glfw-3.4
if(NOT TARGET glfw) 
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${third_party_folder}/glfw)
endif()

# GLM 1.0.1
if(NOT TARGET glm::glm)  
    option(GLM_BUILD_TESTS "" OFF)              # Disable test build
    option(GLM_BUILD_LIBRARY "" OFF)            # Disable library build（header-only）
    option(GLM_ENABLE_CXX_17 "" ON)             # Enable C++20 support
    option(GLM_ENABLE_SIMD_AVX2 "" ON)          # Enable AVX2 instruction set
    option(GLM_ENABLE_FAST_MATH "" ON)          # Enable fast math

    add_subdirectory(glm)
    set_target_properties(glm PROPERTIES FOLDER ${third_party_folder}/glm)
    message(STATUS "GLM configured: C++20=${GLM_ENABLE_CXX_20}, AVX2=${GLM_ENABLE_SIMD_AVX2}")
endif()

# volk
set(VOLK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/volk")
set(VOLK_FILES
    "${VOLK_DIR}/volk.c"
    "${VOLK_DIR}/volk.h")

add_library(volk STATIC ${VOLK_FILES})
set_target_properties(volk PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(volk PUBLIC vulkan)

target_include_directories(volk SYSTEM PUBLIC ${VOLK_DIR})
if (VKB_WSI_SELECTION STREQUAL XCB)
    target_include_directories(volk SYSTEM PUBLIC ${XCB_INCLUDE_DIRS})
elseif (VKB_WSI_SELECTION STREQUAL XLIB)
    target_include_directories(volk SYSTEM PUBLIC ${X11_INCLUDE_DIRS})
elseif (VKB_WSI_SELECTION STREQUAL WAYLAND)
    target_include_directories(volk SYSTEM PUBLIC ${WAYLAND_INCLUDE_DIRS})
endif()

set_property(TARGET volk PROPERTY FOLDER ${third_party_folder}/volk)

# imgui
if(NOT TARGET imgui)
    add_subdirectory(imgui)
    target_compile_definitions(imgui PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES)

    target_link_libraries(imgui PUBLIC volk)
    
    set_target_properties(imgui PROPERTIES FOLDER ${third_party_folder}/imgui)
endif()

# ktx
if(NOT TARGET ktx)
    add_subdirectory(ktx)
    target_link_libraries(ktx PRIVATE volk)
    set_target_properties(ktx PROPERTIES FOLDER ${third_party_folder}/ktx)
endif()

# spdlog
if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "Build SPDLOG examples" OFF)
    option(SPDLOG_INSTALL "Install SPDLOG" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_folder}/spdlog)
endif()

# spirv-cross
add_subdirectory(spirv-cross)

set_target_properties(spirv-cross PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-c PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-core PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-glsl PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-cpp PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-hlsl PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-msl PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-reflect PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-util PROPERTIES FOLDER ${third_party_folder}/spirv-cross POSITION_INDEPENDENT_CODE ON)

# rttr
# set(BUILD_STATIC ON CACHE BOOL "Build RTTR as static library")
# set(BUILD_RTTR_DYNAMIC OFF CACHE BOOL "Disable dynamic library build")
# add_subdirectory(rttr)
# set_target_properties(rttr_core_lib PROPERTIES FOLDER ${third_party_folder}/rttr)

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/include)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma SYSTEM INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE vulkan)

# eventpp
set(EVENTPP_INSTALL OFF CACHE BOOL "Disable eventpp installation")
add_subdirectory(eventpp)