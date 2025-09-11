set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

## No Python parsing; generator handles dependencies via --module-dir

# Run Logos C++ generator on metadata before compilation
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
add_custom_target(run_cpp_generator_waku
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator on ${METADATA_JSON} with module-dir ${PLUGINS_OUTPUT_DIR}"
    VERBATIM
)
add_dependencies(run_cpp_generator_waku cpp_generator_build)

# No separate per-dependency loop needed; handled by --module-dir

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

# Plugin sources
set(PLUGIN_SOURCES
    waku_module_plugin.cpp
    waku_module_plugin.h
    waku_module_interface.h
    ${CMAKE_SOURCE_DIR}/../core/interface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_consumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_consumer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_provider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_provider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/token_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/token_manager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.h
)

# Create the plugin library
add_library(waku_module_plugin SHARED ${PLUGIN_SOURCES})

## Do not compile generated SDK wrappers here; consumer modules include logos_sdk.cpp

# Set output name without lib prefix
set_target_properties(waku_module_plugin PROPERTIES
    PREFIX "")

## No extra dependency printing

# Ensure generator runs before building the plugin
add_dependencies(waku_module_plugin run_cpp_generator_waku)

# Link Qt libraries and libwaku
target_link_libraries(waku_module_plugin PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    ${LIBWAKU_PATH}
)

# Include directories
target_include_directories(waku_module_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/generated
)

# Set common properties for both platforms
set_target_properties(waku_module_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(waku_module_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    # First, copy libwaku.so to the modules directory
    add_custom_command(TARGET waku_module_plugin PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy
        ${LIBWAKU_PATH}
        ${CMAKE_BINARY_DIR}/modules/libwaku.so
        COMMENT "Copying libwaku.so to modules directory"
    )
    
    # Then update the library paths
    add_custom_command(TARGET waku_module_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/waku_module_plugin.dylib" $<TARGET_FILE:waku_module_plugin>
        COMMAND install_name_tool -change "${LIBWAKU_PATH}" "@rpath/libwaku.so" $<TARGET_FILE:waku_module_plugin>
        COMMAND install_name_tool -id "@rpath/libwaku.so" "${CMAKE_BINARY_DIR}/modules/libwaku.so"
        COMMENT "Updating library paths for macOS"
    )
else()
    # First, copy libwaku.so to the modules directory
    add_custom_command(TARGET waku_module_plugin PRE_LINK
        COMMAND ${CMAKE_COMMAND} -E copy
        ${LIBWAKU_PATH}
        ${CMAKE_BINARY_DIR}/modules/libwaku.so
        COMMENT "Copying libwaku.so to modules directory"
    )
    
    # Linux specific settings
    set_target_properties(waku_module_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 