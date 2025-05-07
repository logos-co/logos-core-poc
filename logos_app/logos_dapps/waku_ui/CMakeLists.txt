cmake_minimum_required(VERSION 3.16)
project(WakuUIPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

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

# Check for logos_core library
find_library(LOGOS_CORE_LIB logos_core PATHS ${CMAKE_BINARY_DIR}/../core/lib)

set(SOURCES
    WakuUIComponent.cpp
    WakuUIWidget.cpp
)

add_library(waku_ui SHARED ${SOURCES})

# Add include directories before target_link_libraries
target_include_directories(waku_ui PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/src
)

target_link_libraries(waku_ui 
    PRIVATE 
        Qt6::Core 
        Qt6::Widgets
        component-interfaces
)

# Link against logos_core if found
if(LOGOS_CORE_LIB)
    target_link_libraries(waku_ui PRIVATE ${LOGOS_CORE_LIB})
endif()

# For macOS, we need to set the bundle properties
if(APPLE)
    set_target_properties(waku_ui PROPERTIES
        BUNDLE FALSE
        FRAMEWORK FALSE
        PREFIX ""
        SUFFIX ".dylib"
    )
else()
    set_target_properties(waku_ui PROPERTIES
        PREFIX ""
        SUFFIX ".so"
    )
endif() 