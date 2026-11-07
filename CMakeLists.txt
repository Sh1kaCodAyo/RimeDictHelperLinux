cmake_minimum_required(VERSION 3.15)
project(RimeDictHelperLinux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 PkgConfig 查找 gtkmm
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# 添加可执行文件
add_executable(RimeDictHelperLinux src/main.cpp)

# 包含头文件目录并链接库
target_include_directories(RimeDictHelperLinux PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(RimeDictHelperLinux PRIVATE ${GTKMM_LIBRARY_DIRS})
target_link_libraries(RimeDictHelperLinux PRIVATE ${GTKMM_LIBRARIES})