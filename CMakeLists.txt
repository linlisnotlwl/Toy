cmake_minimum_required(VERSION 3.0)

project(Toy VERSION 0.1)

message(STATUS "TOY This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "TOY This is SOURCE dir " ${PROJECT_SOURCE_DIR})

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(True)
    set(CMAKE_BUILD_TYPE "Debug")
else()
    #set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")

#add_subdirectory(toy)

add_subdirectory(tests)