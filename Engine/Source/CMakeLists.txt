
set(TARGET_NAME CyRRuntime)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Runtime/Physical)
add_subdirectory(Runtime/Render)

set(RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Runtime" CACHE PATH "运行时")
