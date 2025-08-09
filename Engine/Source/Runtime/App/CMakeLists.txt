set(TARGET_NAME RuntimeApp)

Include(${CMAKE_DIR}/LibBase.cmake)

target_include_directories(${TARGET_NAME} PUBLIC 
    ${THIRD_PARTY_DIR}/glfw/include
    ${RUNTIME_DIR}/Core/Include
)

target_link_libraries(${TARGET_NAME} PUBLIC RuntimeCore Render imgui)

target_compile_definitions(${TARGET_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK at: $ENV{VULKAN_SDK}")
    target_include_directories(${TARGET_NAME} PRIVATE "$ENV{VULKAN_SDK}/include")
else()
    message(FATAL_ERROR "VULKAN_SDK environment variable not set!")
endif()

set_property(TARGET ${TARGET_NAME} PROPERTY VS_GLOBAL_DisableExternalDependencies true)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Runtime")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "Source" FILES ${LIB_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Include PREFIX "Include" FILES ${LIB_HEADERS})