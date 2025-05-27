cmake_minimum_required(VERSION 3.16)
project(MainUIPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets RemoteObjects)

# Try to find the component-interfaces package first
find_package(component-interfaces QUIET)

# If not found, use the app/interfaces folder
if(NOT component-interfaces_FOUND)
    # Include the app/interfaces directory
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../app/interfaces)
    
    # Create a component-interfaces library
    add_library(component-interfaces INTERFACE)
    target_include_directories(component-interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../app/interfaces)
endif()

set(SOURCES
    src/main_ui_plugin.cpp
    src/mainwindow.cpp
    src/sidebarbutton.cpp
    src/mdichild.cpp
    src/mdiview.cpp
    src/modulesview.cpp
    src/modulesgenericview.cpp
    src/dashboardview.cpp
    src/coremoduleview.cpp
    src/pluginmethodsview.cpp
    src/packagemanagerview.cpp
    src/main_ui_resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../SDK/cpp/logos_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../SDK/cpp/logos_api.h
)

add_library(main_ui SHARED ${SOURCES})

# Add include directories before target_link_libraries
target_include_directories(main_ui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../SDK/cpp
)

# Add Qt definitions
target_compile_definitions(main_ui PRIVATE
    QT_PLUGIN
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(main_ui 
    PRIVATE 
        Qt6::Core 
        Qt6::Widgets
        Qt6::RemoteObjects
        component-interfaces
)

# For macOS, we need to set the bundle properties
if(APPLE)
    set_target_properties(main_ui PROPERTIES
        BUNDLE FALSE
        FRAMEWORK FALSE
        PREFIX ""
        SUFFIX ".dylib"
    )
else()
    set_target_properties(main_ui PROPERTIES
        PREFIX ""
        SUFFIX ".so"
    )
endif() 