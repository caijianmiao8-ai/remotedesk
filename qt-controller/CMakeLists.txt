cmake_minimum_required(VERSION 3.21)

project(Controller VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets Network WebSockets Multimedia)
find_package(datachannel CONFIG REQUIRED)

add_executable(Controller
    src/controller/App.cpp
    src/controller/UiMainWindow.cpp
    src/controller/AuthClient.cpp
    src/controller/SignalingClient.cpp
    src/controller/WebRtcPeer.cpp
)

target_include_directories(Controller
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

qt_standard_project_setup()

set_target_properties(Controller PROPERTIES
    OUTPUT_NAME "Controller"
)

find_package(OpenSSL REQUIRED)

# Link against Qt and libdatachannel

if(TARGET datachannel::datachannel)
    target_link_libraries(Controller PRIVATE datachannel::datachannel)
else()
    target_link_libraries(Controller PRIVATE datachannel)
endif()

target_link_libraries(Controller PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    OpenSSL::SSL
    OpenSSL::Crypto
)

install(TARGETS Controller)
