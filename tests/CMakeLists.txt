message(STATUS "tests This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "tests This is SOURCE dir " ${PROJECT_SOURCE_DIR})

# 定义工程根目录; CMAKE_SOURCE_DIR为内建变量，表示工程根目录的CMakeLists.txt文件路径
SET(ROOT_DIR ${CMAKE_SOURCE_DIR})

# 指定头文件搜索路径
INCLUDE_DIRECTORIES(${ROOT_DIR}/lib)

# 指定引用的外部库的搜索路径
LINK_DIRECTORIES(${ROOT_DIR}/lib)

# 查找当前目录下的所有源文件
aux_source_directory(../toy DIR_SRCS)

include_directories ("${PROJECT_SOURCE_DIR}/toy")

#set(SRC_LIST test_log.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# -g3 可以调试宏定义
#add_compile_options(-std=c++11 -pthread -Wall -O0 -g3)
find_package (Threads)


add_executable(toy_log.out test_log.cpp ${DIR_SRCS})
target_link_libraries (toy_log.out ${CMAKE_THREAD_LIBS_INIT})

#add_executable(toy_json.out test_json.cpp ${DIR_SRCS})
#target_link_libraries(toy_json.out libtoyjson.a)

#add_executable(toy_buffer.out test_buffer.cpp ${DIR_SRCS})
#target_link_libraries (toy_buffer.out ${CMAKE_THREAD_LIBS_INIT})