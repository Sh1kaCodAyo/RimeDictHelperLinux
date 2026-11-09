cmake_minimum_required(VERSION 3.15)
project(RimeDictHelperLinux LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用 PkgConfig 查找 gtkmm
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# 添加可执行文件
add_executable(RimeDictHelperLinux src/main.cpp
        src/ConfigManager.h
        src/ConfigManager.cpp
#        src/AppData.h
        src/DictEntry.h
        src/DictEntry.cpp)

# 包含头文件目录并链接库
target_include_directories(RimeDictHelperLinux PRIVATE ${GTKMM_INCLUDE_DIRS})
target_link_directories(RimeDictHelperLinux PRIVATE ${GTKMM_LIBRARY_DIRS})
target_link_libraries(RimeDictHelperLinux PRIVATE ${GTKMM_LIBRARIES})

# 将 resource/deploy.sh 复制到构建目录（与可执行文件同级）
configure_file(
        ${CMAKE_SOURCE_DIR}/resource/deploy.sh
        ${CMAKE_BINARY_DIR}/deploy.sh
        COPYONLY
)
execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/deploy.sh)
file(COPY ${CMAKE_SOURCE_DIR}/resource/RimeDictHelper2.png DESTINATION ${CMAKE_BINARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()