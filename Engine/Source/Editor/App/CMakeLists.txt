set(TARGET_NAME CyREngineEditor)

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

add_executable(${TARGET_NAME} ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(${TARGET_NAME} PUBLIC 
    ${RUNTIME_DIR}
    ${EDITOR_DIR}
)