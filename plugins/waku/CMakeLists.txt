cmake_minimum_required(VERSION 3.10)
project(waku)

# Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core REQUIRED)
endif()

# Set automoc on
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find the libwaku library
find_library(LIBWAKU_PATH
    NAMES libwaku.so
    PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT LIBWAKU_PATH)
    message(FATAL_ERROR "libwaku.so not found in lib/ directory")
endif()

# Add the library
add_library(waku SHARED
    waku.cpp
    waku.h
    waku_interface.h
)

# Set output name without lib prefix and with _plugin postfix
set_target_properties(waku PROPERTIES
    PREFIX ""
    OUTPUT_NAME "waku_plugin")

# Link against libwaku and Qt
target_link_libraries(waku PRIVATE 
    ${LIBWAKU_PATH}
    Qt::Core
)

# Include directories
target_include_directories(waku PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Set common properties for both platforms
set_target_properties(waku PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(waku PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    # First, copy libwaku.so to the plugins directory
    add_custom_command(TARGET waku PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy
        ${LIBWAKU_PATH}
        ${CMAKE_BINARY_DIR}/plugins/libwaku.so
        COMMENT "Copying libwaku.so to plugins directory"
    )

    # Then update the library paths
    add_custom_command(TARGET waku POST_BUILD
        COMMAND install_name_tool -id "@rpath/waku_plugin.dylib" $<TARGET_FILE:waku>
        COMMAND install_name_tool -change "${LIBWAKU_PATH}" "@rpath/libwaku.so" $<TARGET_FILE:waku>
        COMMAND install_name_tool -id "@rpath/libwaku.so" "${CMAKE_BINARY_DIR}/plugins/libwaku.so"
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(waku PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 