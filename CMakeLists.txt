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
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address") #AddressSanitizer
set(TSAN_FLAGS "-fsanitize=thread -fPIE -pie -fno-omit-frame-pointer") #ThreadSanitizer
set(MSAN_FLAGS "-fsanitize=memory -fsanitize-memory-track-origins -fPIE -pie -fno-omit-frame-pointer") #MemorySanitizer

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -rdynamic -g3 -O0 -Wall -ggdb  ${ASAN_FLAGS}") #${TSAN_FLAGS}
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -rdynamic -O2 -Wall -DNDEBUG ")

#add_subdirectory(toy)

add_subdirectory(tests)