cmake_minimum_required(VERSION 3.14)
project(LogosSDK)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core RemoteObjects)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# SDK sources
set(SDK_SOURCES
    logos_api.cpp
    logos_api.h
    logos_api_client.cpp
    logos_api_client.h
    logos_api_consumer.cpp
    logos_api_consumer.h
    logos_api_provider.cpp
    logos_api_provider.h
    module_proxy.cpp
    module_proxy.h
    token_manager.cpp
    token_manager.h
)

# Create the SDK library as STATIC instead of SHARED
add_library(logos_sdk STATIC ${SDK_SOURCES})

# Link Qt libraries
target_link_libraries(logos_sdk PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::RemoteObjects)

# Include directories
target_include_directories(logos_sdk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set output directories for static library
set_target_properties(logos_sdk PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
) 