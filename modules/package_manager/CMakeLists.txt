set(CMAKE_AUTOMOC ON)

# Plugin sources
set(PLUGIN_SOURCES
    package_manager_plugin.cpp
    package_manager_plugin.h
    package_manager_interface.h
    ${CMAKE_SOURCE_DIR}/../core/interface.h
)

# Create the plugin library
add_library(package_manager_plugin SHARED ${PLUGIN_SOURCES})

# Set output name without lib prefix
set_target_properties(package_manager_plugin PROPERTIES
    PREFIX "")

# Link Qt libraries
target_link_libraries(package_manager_plugin PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# Include directories
target_include_directories(package_manager_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../core
)

# Set common properties for both platforms
set_target_properties(package_manager_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(package_manager_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    add_custom_command(TARGET package_manager_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/package_manager_plugin.dylib" $<TARGET_FILE:package_manager_plugin>
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(package_manager_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 