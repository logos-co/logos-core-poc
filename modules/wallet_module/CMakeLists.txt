set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

## No Python parsing; generator handles dependencies via --module-dir

# Run Logos C++ generator on metadata before compilation
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
add_custom_target(run_cpp_generator_wallet
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator on ${METADATA_JSON} with module-dir ${PLUGINS_OUTPUT_DIR}"
    VERBATIM
)
add_dependencies(run_cpp_generator_wallet cpp_generator_build)

# Find the wallet C library built from go-wallet-sdk
find_library(LIBWALLET_PATH
    NAMES libgowalletsdk.so libgowalletsdk.dylib libgowalletsdk.dll
    PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT LIBWALLET_PATH)
    message(WARNING "Wallet C library not found in lib/. Build it via build_wallet_lib.sh before linking.")
endif()

# Plugin sources
set(PLUGIN_SOURCES
    wallet_module_plugin.cpp
    wallet_module_plugin.h
    wallet_module_interface.h
    ${CMAKE_SOURCE_DIR}/../logos-liblogos/interface.h
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
add_library(wallet_module_plugin SHARED ${PLUGIN_SOURCES})

## Do not compile generated SDK wrappers here; consumer modules include logos_sdk.cpp

# Set output name without lib prefix
set_target_properties(wallet_module_plugin PROPERTIES
    PREFIX "")

## No extra dependency printing

# Link Qt libraries and the wallet C library if present
target_link_libraries(wallet_module_plugin PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)
if(LIBWALLET_PATH)
    target_link_libraries(wallet_module_plugin PRIVATE ${LIBWALLET_PATH})
endif()

# Include directories
target_include_directories(wallet_module_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/../logos-liblogos
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated
)

# Set common properties for both platforms
set_target_properties(wallet_module_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(wallet_module_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)

    # If libgowalletsdk exists, copy and fix install names
    if(LIBWALLET_PATH)
        add_custom_command(TARGET wallet_module_plugin PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIBWALLET_PATH}
            ${CMAKE_BINARY_DIR}/modules/libgowalletsdk.dylib
            COMMENT "Copying libgowalletsdk to modules directory"
            VERBATIM
        )

        add_custom_command(TARGET wallet_module_plugin POST_BUILD
            COMMAND install_name_tool -id "@rpath/wallet_module_plugin.dylib" $<TARGET_FILE:wallet_module_plugin>
            COMMAND ${CMAKE_COMMAND} -DWALLET_LIB_PATH=${CMAKE_BINARY_DIR}/modules/libgowalletsdk.dylib -DPLUGIN_PATH=${CMAKE_BINARY_DIR}/modules/wallet_module_plugin.dylib -P ${CMAKE_CURRENT_SOURCE_DIR}/fix_wallet_install_name.cmake
            COMMENT "Updating library paths for macOS"
        )
    else()
        add_custom_command(TARGET wallet_module_plugin POST_BUILD
            COMMAND install_name_tool -id "@rpath/wallet_module_plugin.dylib" $<TARGET_FILE:wallet_module_plugin>
            COMMENT "Updating library paths for macOS (wallet library not found)"
        )
    endif()
else()
    # Linux specific settings
    set_target_properties(wallet_module_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)

    # First, copy libgowallet.so to the modules directory if present
    if(LIBWALLET_PATH)
        add_custom_command(TARGET wallet_module_plugin PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIBWALLET_PATH}
            ${CMAKE_BINARY_DIR}/modules/libgowalletsdk.so
            COMMENT "Copying libgowalletsdk.so to modules directory"
        )
    endif()
endif()


# No separate per-dependency loop needed; handled by --module-dir

# Ensure generator runs before building the plugin
add_dependencies(wallet_module_plugin run_cpp_generator_wallet)
