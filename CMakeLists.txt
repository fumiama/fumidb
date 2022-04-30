cmake_minimum_required(VERSION 3.0.0)
project(fumidb VERSION 1.0)
SET(CMAKE_BUILD_TYPE "Release")

include(TestBigEndian)
test_big_endian(isBigEndian)
if(${isBigEndian})
    add_definitions(-DWORDS_BIGENDIAN)
endif()

include(option.txt)

if(${BUILD_TEST})
    message(STATUS "Building tests...")
    enable_testing()
    add_subdirectory(tests)
endif()