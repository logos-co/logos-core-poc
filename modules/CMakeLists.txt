cmake_minimum_required(VERSION 3.14)
project(LogosModules)

# Find Qt packages
find_package(Qt6 COMPONENTS Core REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core REQUIRED)
endif()

# Set QT_VERSION_MAJOR based on what was found
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
endif()

# Look for the logos_core library
find_library(LOGOS_CORE_LIB logos_core PATHS 
    ${CMAKE_BINARY_DIR}/../core/build/lib 
    ${CMAKE_BINARY_DIR}/lib
)

# Message about logos_core library status
if(LOGOS_CORE_LIB)
    message(STATUS "Found logos_core library: ${LOGOS_CORE_LIB}")
else()
    message(WARNING "logos_core library not found. Plugins may not link correctly.")
endif()

# Add subdirectories for each plugin
add_subdirectory(calculator)
add_subdirectory(hello_world)
add_subdirectory(waku)
add_subdirectory(chat)
add_subdirectory(template_module)
add_subdirectory(package_manager)