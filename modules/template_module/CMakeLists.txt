set(CMAKE_AUTOMOC ON)

# Plugin sources
set(PLUGIN_SOURCES
    template_module_plugin.cpp
    template_module_plugin.h
    template_module_interface.h
    ${CMAKE_SOURCE_DIR}/../core/interface.h
)

# Create the plugin library
add_library(template_module_plugin SHARED ${PLUGIN_SOURCES})

# Set output name without lib prefix
set_target_properties(template_module_plugin PROPERTIES
    PREFIX "")

# Link Qt libraries
target_link_libraries(template_module_plugin PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Include directories
target_include_directories(template_module_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../core
)

# Set common properties for both platforms
set_target_properties(template_module_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(template_module_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    add_custom_command(TARGET template_module_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/template_module_plugin.dylib" $<TARGET_FILE:template_module_plugin>
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(template_module_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 