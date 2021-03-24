cmake_minimum_required(VERSION 3.17)

project(OpenAL)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

# Add source files
file(GLOB_RECURSE SOURCE_FILES
        src/*.c
        src/*.cpp)

#### Lib
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

LINK_DIRECTORIES(lib)
LINK_DIRECTORIES(third_party/lib)

include_directories(third_party/include)
set(LIB OpenAL32 SndFile)

add_executable(OpenAL ${SOURCE_FILES})
target_link_libraries(OpenAL ${LIB})