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