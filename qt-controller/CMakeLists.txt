cmake_minimum_required(VERSION 3.21)
project(Controller LANGUAGES CXX)

# ==== Qt 基本开关 ====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==== Qt 组件 ====
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets Multimedia)

# ==== 第三方依赖（通过 vcpkg） ====
# vcpkg: libdatachannel, openssl, usrsctp, libsrtp, libyuv, opus, openh264
find_package(OpenSSL REQUIRED)
find_package(datachannel CONFIG REQUIRED)   # vcpkg 安装: libdatachannel[openssl,libjuice,usrsctp]

# libdatachannel 导出的 CMake target 在不同包名下可能不同，这里做兼容探测
set(LIBDATACHANNEL_TARGET "")
if (TARGET rtc)
    set(LIBDATACHANNEL_TARGET rtc)
elseif (TARGET rtc::rtc)
    set(LIBDATACHANNEL_TARGET rtc::rtc)
elseif (TARGET datachannel::rtc)
    set(LIBDATACHANNEL_TARGET datachannel::rtc)
else()
    message(FATAL_ERROR "libdatachannel CMake target not found. Known names: rtc / rtc::rtc / datachannel::rtc")
endif()

# ==== 源码收集 ====
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    src/*.cpp
    include/*.h
    res/*.qrc
)

# ==== 可执行文件 ====
add_executable(Controller ${SRC})

target_include_directories(Controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ==== 链接库 ====
target_link_libraries(Controller PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::WebSockets Qt6::Multimedia
    ${LIBDATACHANNEL_TARGET}
    OpenSSL::SSL OpenSSL::Crypto
)

if (WIN32)
    # Windows 上 socket 需要
    target_link_libraries(Controller PRIVATE ws2_32)
endif()

# ==== 更友好的输出目录 ====
set_target_properties(Controller PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ==== 构建提示 ====
message(STATUS "Using libdatachannel target: ${LIBDATACHANNEL_TARGET}")
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
