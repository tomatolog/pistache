cmake_minimum_required (VERSION 3.0.2)
project (pistache)
include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
if(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

include_directories(/usr/local/include)
include_directories(/rocksplicator)
include_directories(/rocksplicator/build)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/include)
add_subdirectory (src)

include_directories (src)

add_subdirectory (examples)
