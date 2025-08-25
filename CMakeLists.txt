# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(AsyncLogSystem CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
add_compile_options(-g -Wno-register -D_GLIBCXX_USE_OPEN_MODE -DBOOST_BIND_GLOBAL_PLACEHOLDERS)

# 查找依赖库
# 查找 Boost 库
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
include_directories(${Boost_INCLUDE_DIRS})

# 查找其他系统库
find_package(Threads REQUIRED)

# 假设 jsoncpp 和 bundle、event 库在系统路径中
# 如果需要特定版本或路径，可能需要调整
find_library(JSONCPP_LIBRARY jsoncpp)
find_library(BUNDLE_LIBRARY bundle)
find_library(EVENT_LIBRARY event)

# 添加头文件目录
include_directories(include)
include_directories(../log)

# 收集所有源文件
file(GLOB SRCS "*.cpp")

# 排除不需要的源文件（如果有的话）
# 这里没有明确排除的文件，但保留结构以便需要时使用
# list(REMOVE_ITEM SRCS "${CMAKE_CURRENT_SOURCE_DIR}/text.cpp")

# 创建主程序可执行文件
add_executable(test ${SRCS})
# 链接库
target_link_libraries(test
    Threads::Threads
    stdc++fs
    ${BUNDLE_LIBRARY}
    ${EVENT_LIBRARY}
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# 创建调试目标（排除 text.o）
# 首先，找出除了 text.cpp 之外的所有源文件
file(GLOB GDB_SRCS "*.cpp")
list(REMOVE_ITEM GDB_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/text.cpp")

add_executable(gdb_test ${GDB_SRCS})
# 链接库
target_link_libraries(gdb_test
    Threads::Threads
    stdc++fs
    ${BUNDLE_LIBRARY}
    ${EVENT_LIBRARY}
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARY}
)

# 设置默认目标
add_custom_target(build_all DEPENDS test gdb_test)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_BINARY_DIR}/deep_storage;${CMAKE_CURRENT_BINARY_DIR}/low_storage;${CMAKE_CURRENT_BINARY_DIR}/logfile;${CMAKE_CURRENT_BINARY_DIR}/storage.data")

# 清理目标（覆盖默认的 clean 目标）
add_custom_target(clean_objs
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_CURRENT_BINARY_DIR}/*.o"
)

# 设置依赖关系
add_dependencies(test clean_objs)
add_dependencies(gdb_test clean_objs)