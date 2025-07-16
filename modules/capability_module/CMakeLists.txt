set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Plugin sources
set(PLUGIN_SOURCES
    capability_module_plugin.cpp
    capability_module_plugin.h
    capability_module_interface.h
    ${CMAKE_SOURCE_DIR}/../core/interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_consumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_consumer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.h
)

# Create the plugin library
add_library(capability_module_plugin SHARED ${PLUGIN_SOURCES})

# Set output name without lib prefix
set_target_properties(capability_module_plugin PROPERTIES
    PREFIX "")

# Link Qt libraries
target_link_libraries(capability_module_plugin PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)

# Include directories
target_include_directories(capability_module_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp
)

# Set common properties for both platforms
set_target_properties(capability_module_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(capability_module_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    add_custom_command(TARGET capability_module_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/capability_module_plugin.dylib" $<TARGET_FILE:capability_module_plugin>
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(capability_module_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 