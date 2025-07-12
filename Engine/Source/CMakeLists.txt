set(TARGET_NAME CyRRuntime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Obtain all subdirectories under the Runtime directory
file(GLOB RUNTIME_MODULES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Runtime/*")

foreach(DIR ${RUNTIME_MODULES})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}/CMakeLists.txt")
            message(STATUS "Adding subdirectory: ${DIR}")
            add_subdirectory("${DIR}")
        endif()
    endif()
endforeach()

set(RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Runtime" CACHE PATH "运行时")
