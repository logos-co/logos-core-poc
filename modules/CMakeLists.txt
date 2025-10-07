cmake_minimum_required(VERSION 3.14)
project(LogosModules)

# Find Qt packages
find_package(Qt6 COMPONENTS Core RemoteObjects REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core RemoteObjects REQUIRED)
endif()

# Set QT_VERSION_MAJOR based on what was found
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
endif()

# Ensure cpp-generator is built before building modules
set(CPP_GENERATOR_COMPILE_SCRIPT "${CMAKE_SOURCE_DIR}/../logos-cpp-sdk/cpp-generator/compile.sh")
add_custom_target(cpp_generator_build
    COMMAND bash "${CPP_GENERATOR_COMPILE_SCRIPT}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/.."
    COMMENT "Building logos-cpp-generator via logos-cpp-sdk/cpp-generator/compile.sh"
    VERBATIM
)

# Look for the logos_core library
find_library(LOGOS_CORE_LIB logos_core PATHS 
    ${CMAKE_BINARY_DIR}/../logos-liblogos/build/lib 
    ${CMAKE_BINARY_DIR}/lib
)

# Message about logos_core library status
if(LOGOS_CORE_LIB)
    message(STATUS "Found logos_core library: ${LOGOS_CORE_LIB}")
else()
    message(WARNING "logos_core library not found. Plugins may not link correctly.")
endif()

# Add subdirectories for each plugin
add_subdirectory(capability_module)
add_subdirectory(package_manager)
add_subdirectory(waku_module)
add_subdirectory(chat)
add_subdirectory(logos_irc)
add_subdirectory(template_module)
add_subdirectory(simple_module)
add_subdirectory(wallet_module)

# Make all module plugin targets depend on cpp_generator_build
add_dependencies(capability_module_plugin cpp_generator_build)
add_dependencies(package_manager_plugin cpp_generator_build)
add_dependencies(waku_module_plugin cpp_generator_build)
add_dependencies(chat_module_plugin cpp_generator_build)
add_dependencies(logos_irc_plugin cpp_generator_build)
add_dependencies(template_module_plugin cpp_generator_build)
add_dependencies(simple_module_plugin cpp_generator_build)
add_dependencies(wallet_module_plugin cpp_generator_build)
