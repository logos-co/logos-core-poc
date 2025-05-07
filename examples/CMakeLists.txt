cmake_minimum_required(VERSION 3.14)
project(LogosCoreExamples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Find the logos_core library
find_library(LOGOS_CORE_LIBRARY logos_core PATHS ${CMAKE_SOURCE_DIR}/../core/build/lib REQUIRED)
message(STATUS "Found logos_core library: ${LOGOS_CORE_LIBRARY}")

# Add include directories for logos_core
include_directories(${CMAKE_SOURCE_DIR}/../core/src)
include_directories(${CMAKE_SOURCE_DIR}/../core)

# Add subdirectories
add_subdirectory(custom_app) 