cmake_minimum_required(VERSION 3.16)
project(QtRemoteObjectRegistryExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets RemoteObjects REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets RemoteObjects REQUIRED)
endif()

# Add subdirectories for each component
add_subdirectory(registry)
add_subdirectory(source)
add_subdirectory(replica) 