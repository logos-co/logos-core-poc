set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI) and Network (needed for IRC server)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects Network)

# Print dependencies from metadata.json during configure (using generator)
#set(METADATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
#if(EXISTS "${METADATA_FILE}" AND EXISTS "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
#    execute_process(
#        COMMAND "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator" --metadata "${METADATA_FILE}"
#        OUTPUT_VARIABLE METADATA_DEPS_OUTPUT
#        ERROR_VARIABLE METADATA_DEPS_ERROR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#    )
#    if(METADATA_DEPS_OUTPUT)
#        string(REPLACE "\n" ";" METADATA_DEPS_LIST "${METADATA_DEPS_OUTPUT}")
#        foreach(dep IN LISTS METADATA_DEPS_LIST)
#            message(STATUS "${dep}")
#        endforeach()
#    endif()
#else()
#    message(WARNING "Generator or metadata.json not found; skipping dependency printout")
#endif()

# (We will include the umbrella source logos_sdk.cpp instead of globbing individual files)

# Run Logos C++ generator on metadata before compilation
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")

set(GENERATED_LOGOS_SDK_CPP ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp)
set_source_files_properties(${GENERATED_LOGOS_SDK_CPP} PROPERTIES GENERATED TRUE)
if(APPLE)
    set(PLUGIN_SUFFIX ".dylib")
elseif(WIN32)
    set(PLUGIN_SUFFIX ".dll")
else()
    set(PLUGIN_SUFFIX ".so")
endif()

# Build dependency plugin file list from generator-reported metadata deps for proper build ordering
set(DEPENDENCY_PLUGIN_PATHS)
if(METADATA_DEPS_LIST)
    foreach(dep IN LISTS METADATA_DEPS_LIST)
        list(APPEND DEPENDENCY_PLUGIN_PATHS ${PLUGINS_OUTPUT_DIR}/${dep}_plugin${PLUGIN_SUFFIX})
    endforeach()
endif()

add_custom_target(generate_irc_sdk_wrappers
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    WORKING_DIRECTORY "${REPO_ROOT}"
    DEPENDS ${METADATA_JSON}
    COMMENT "Generating umbrella ${GENERATED_LOGOS_SDK_CPP} and wrappers via logos-cpp-generator"
    VERBATIM
)
add_dependencies(generate_irc_sdk_wrappers cpp_generator_build)
# Dynamically depend on CMake targets for modules listed in metadata.json
# (Not required because generator handles ordering via --module-dir)
# if(METADATA_DEPS_LIST)
#     foreach(dep IN LISTS METADATA_DEPS_LIST)
#         if(TARGET ${dep})
#             add_dependencies(generate_irc_sdk_wrappers ${dep})
#         endif()
#     endforeach()
# endif()

# No separate per-dependency loop needed; handled by --module-dir

# Plugin sources
set(PLUGIN_SOURCES
    logos_irc_plugin.cpp
    logos_irc_plugin.h
    logos_irc_interface.h
    ircserver.cpp
    ircserver.h
    ircclient.cpp
    ircclient.h
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
add_library(logos_irc_plugin SHARED ${PLUGIN_SOURCES})

# Ensure generator runs before building the plugin
add_dependencies(logos_irc_plugin generate_irc_sdk_wrappers)

# Add umbrella source that includes all generated wrappers
target_sources(logos_irc_plugin PRIVATE ${GENERATED_LOGOS_SDK_CPP})

# Set output name without lib prefix
set_target_properties(logos_irc_plugin PROPERTIES
    PREFIX "")

# Link Qt libraries
target_link_libraries(logos_irc_plugin PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    Qt${QT_VERSION_MAJOR}::Network
)

# Include directories
target_include_directories(logos_irc_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../logos-liblogos
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated
)

# Set common properties for both platforms
set_target_properties(logos_irc_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(logos_irc_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
    
    add_custom_command(TARGET logos_irc_plugin POST_BUILD
        COMMAND install_name_tool -id "@rpath/logos_irc_plugin.dylib" $<TARGET_FILE:logos_irc_plugin>
        COMMENT "Updating library paths for macOS"
    )
else()
    # Linux specific settings
    set_target_properties(logos_irc_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif() 