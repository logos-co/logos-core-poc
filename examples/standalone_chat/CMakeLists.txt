cmake_minimum_required(VERSION 3.16)
project(QtHelloWorld VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Widgets REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets REQUIRED)
endif()

# Include directories for the project
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos_app/app/interfaces
)

# Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add the executable
add_executable(standalone_chat
    main.cpp
    mainwindow.cpp
    mainwindow.h
    core_manager.cpp
    core_manager.h
)

# Link against the correct Qt version and the core library
if (Qt6_FOUND)
    target_link_libraries(standalone_chat PRIVATE Qt6::Widgets)
else()
    target_link_libraries(standalone_chat PRIVATE Qt5::Widgets)
endif()

# Determine library extension based on platform
if(APPLE)
    set(LIB_EXTENSION "dylib")
else()
    set(LIB_EXTENSION "so")
endif()

# Set the path to the logos_core library
set(LOGOS_CORE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/lib/liblogos_core.${LIB_EXTENSION}")

# Check if the library exists
if(NOT EXISTS ${LOGOS_CORE_LIB})
    message(WARNING "LogosCore library not found at ${LOGOS_CORE_LIB}")
    # Try the alternative extension as fallback
    if(APPLE)
        set(LOGOS_CORE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/lib/liblogos_core.so")
    else()
        set(LOGOS_CORE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/lib/liblogos_core.dylib")
    endif()
    if(NOT EXISTS ${LOGOS_CORE_LIB})
        message(FATAL_ERROR "LogosCore library not found with either extension")
    endif()
endif()

# Link against logos_core
target_link_libraries(standalone_chat PRIVATE ${LOGOS_CORE_LIB})

# Copy plugins to the output directory
add_custom_command(TARGET standalone_chat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules
    COMMENT "Creating modules directory"
)

# Copy only specific modules
foreach(PLUGIN_NAME "chat_plugin" "libwaku" "waku_plugin")
    if(APPLE)
        set(PLUGIN_EXT "dylib")
    else()
        set(PLUGIN_EXT "so")
    endif()
    
    set(PLUGIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/modules/${PLUGIN_NAME}.${PLUGIN_EXT}")
    
    # Check if plugin exists with primary extension
    if(NOT EXISTS ${PLUGIN_PATH})
        # Try alternative extension
        if(APPLE)
            set(PLUGIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/modules/${PLUGIN_NAME}.so")
        else()
            set(PLUGIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../core/build/modules/${PLUGIN_NAME}.dylib")
        endif()
    endif()
    
    # Add command to copy the plugin if it exists
    if(EXISTS ${PLUGIN_PATH})
        add_custom_command(TARGET standalone_chat POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
                ${PLUGIN_PATH}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/modules/
            COMMENT "Copying module: ${PLUGIN_NAME}"
        )
    else()
        message(STATUS "Module not found: ${PLUGIN_NAME}")
    endif()
endforeach()

# Copy chat_ui library to the build/bin folder
if(APPLE)
    set(CHAT_UI_EXT "dylib")
else()
    set(CHAT_UI_EXT "so")
endif()

set(CHAT_UI_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../logos_app/logos_dapps/chat_ui/build/chat_ui.${CHAT_UI_EXT}")

# Check if primary extension exists
if(NOT EXISTS ${CHAT_UI_SOURCE})
    # Try alternative extension
    if(APPLE)
        set(CHAT_UI_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../logos_app/logos_dapps/chat_ui/build/chat_ui.so")
    else()
        set(CHAT_UI_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../logos_app/logos_dapps/chat_ui/build/chat_ui.dylib")
    endif()
endif()

# Add command to copy the chat_ui library if it exists
if(EXISTS ${CHAT_UI_SOURCE})
    add_custom_command(TARGET standalone_chat POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CHAT_UI_SOURCE}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
        COMMENT "Copying chat_ui library to bin folder"
    )
    message(STATUS "Will copy chat_ui from: ${CHAT_UI_SOURCE}")
else()
    message(WARNING "Chat UI library not found at: ${CHAT_UI_SOURCE}")
endif()

# Print build information
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "LOGOS_CORE_LIB: ${LOGOS_CORE_LIB}") 