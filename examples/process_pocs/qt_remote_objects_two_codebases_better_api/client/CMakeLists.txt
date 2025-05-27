cmake_minimum_required(VERSION 3.14)
project(qt_remote_objects_client)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Explicitly ignore other potential Qt installations
set(CMAKE_IGNORE_PATH "/Users/iurimatias/.nix-profile" "/opt/homebrew")

# Find required Qt packages
# Set Qt6_DIR explicitly using absolute paths
set(Qt6_DIR "$ENV{QT_DIR}/lib/cmake/Qt6")
set(Qt6Core_DIR "$ENV{QT_DIR}/lib/cmake/Qt6Core")
set(Qt6Widgets_DIR "$ENV{QT_DIR}/lib/cmake/Qt6Widgets")
set(Qt6RemoteObjects_DIR "$ENV{QT_DIR}/lib/cmake/Qt6RemoteObjects")

# Print debug information about Qt paths
message(STATUS "Looking for Qt in: $ENV{QT_DIR}")
message(STATUS "Qt6_DIR set to: ${Qt6_DIR}")
message(STATUS "Qt6RemoteObjects_DIR set to: ${Qt6RemoteObjects_DIR}")

# Only search in our custom Qt installation
set(CMAKE_PREFIX_PATH "$ENV{QT_DIR}")

# Find the required Qt packages
find_package(Qt6 COMPONENTS Core Widgets RemoteObjects REQUIRED)

# Client executable
add_executable(counter_client
    client.cpp
    Counter.cpp
)

# Link libraries
target_link_libraries(counter_client PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::RemoteObjects
) 