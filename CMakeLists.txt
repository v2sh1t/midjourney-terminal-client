cmake_minimum_required(VERSION 3.15)
project(midjourney_cli VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置CMake下载代理
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_TLS_VERIFY OFF)
set(CMAKE_TLS_VERIFY OFF)

# 设置代理服务器
set(ENV{HTTP_PROXY} "https://gh.tryxd.cn/")
set(ENV{HTTPS_PROXY} "https://gh.tryxd.cn/")

# 添加外部依赖
include(FetchContent)

# 添加 CPR 库
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://gh.tryxd.cn/https://github.com/libcpr/cpr
    GIT_TAG 1.10.5
)
FetchContent_MakeAvailable(cpr)

# 添加 nlohmann/json
FetchContent_Declare(
    json
    URL https://gh.tryxd.cn/https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# 添加 CLI11
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://gh.tryxd.cn/https://github.com/CLIUtils/CLI11
    GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(CLI11)

# 添加源文件
add_executable(midjourney_cli 
    src/main.cpp
    src/midjourney_client.cpp
    src/config.cpp
)

# 链接依赖库
target_link_libraries(midjourney_cli PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
    CLI11::CLI11
)

# 包含头文件目录
target_include_directories(midjourney_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)