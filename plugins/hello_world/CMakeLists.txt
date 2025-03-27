set(CMAKE_AUTOMOC ON)

# Plugin sources
set(PLUGIN_SOURCES
    hello_world_plugin.cpp
    hello_world_plugin.h
    ${CMAKE_SOURCE_DIR}/../core/interface.h
)

# Create the plugin library
add_library(hello_world_plugin SHARED ${PLUGIN_SOURCES})

# Set output name without lib prefix
set_target_properties(hello_world_plugin PROPERTIES
    PREFIX "")

# Link Qt libraries
target_link_libraries(hello_world_plugin PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Include directories
target_include_directories(hello_world_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../core
)

# Set common properties for both platforms
set_target_properties(hello_world_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(hello_world_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    add_custom_command(TARGET hello_world_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/hello_world_plugin.dylib" $<TARGET_FILE:hello_world_plugin>
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(hello_world_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 