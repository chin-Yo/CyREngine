set(TARGET_NAME Render)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "Source/*.cpp"
    "Source/*.cxx"
    "*.cpp"
)

file(GLOB_RECURSE LIB_HEADERS
    "Include/*.h"
    "Include/*.hpp"
)
#message(STATUS "LIB_HEADERS: ${LIB_HEADERS}")

add_executable(${TARGET_NAME} WIN32 ${LIB_SOURCES} ${LIB_HEADERS})

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

if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK at: $ENV{VULKAN_SDK}")
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
target_link_libraries(${TARGET_NAME} PUBLIC Core)

# Organize the folder structure for the IDE
set_property(TARGET ${TARGET_NAME} PROPERTY VS_GLOBAL_DisableExternalDependencies true)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Runtime")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "Source" FILES ${LIB_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Include PREFIX "Include" FILES ${LIB_HEADERS})
