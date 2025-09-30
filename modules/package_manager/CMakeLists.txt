set(CMAKE_AUTOMOC ON)

## No Python parsing; generator handles dependencies via --module-dir

# Run Logos C++ generator on metadata before compilation
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
add_custom_target(run_cpp_generator_package_manager
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator on ${METADATA_JSON} with module-dir ${PLUGINS_OUTPUT_DIR}"
    VERBATIM
)
add_dependencies(run_cpp_generator_package_manager cpp_generator_build)

# No separate per-dependency loop needed; handled by --module-dir

# Plugin sources
set(PLUGIN_SOURCES
    package_manager_plugin.cpp
    package_manager_plugin.h
    package_manager_interface.h
    ${CMAKE_SOURCE_DIR}/../logos-liblogos/interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_provider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_provider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/token_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/token_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.h
)

# Create the plugin library
add_library(package_manager_plugin SHARED ${PLUGIN_SOURCES})

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp PROPERTIES GENERATED TRUE)

## Do not compile generated SDK wrappers here; consumer modules include logos_sdk.cpp

# Set output name without lib prefix
set_target_properties(package_manager_plugin PROPERTIES
    PREFIX "")

## No extra dependency printing

# Ensure generator runs before building the plugin
add_dependencies(package_manager_plugin run_cpp_generator_package_manager)

# Link Qt libraries
target_link_libraries(package_manager_plugin PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::RemoteObjects)

# Include directories
target_include_directories(package_manager_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../logos-liblogos
    ${CMAKE_SOURCE_DIR}/../logos-cpp-sdk/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated
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
