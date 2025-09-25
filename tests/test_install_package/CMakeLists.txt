set(CMAKE_AUTOMOC ON)

# Run Logos C++ generator to produce typed SDK wrappers for this test
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${REPO_ROOT}/modules/build/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
set(GENERATED_STAMP "${CMAKE_CURRENT_BINARY_DIR}/logos_sdk_generated.stamp")

add_custom_command(
    OUTPUT ${GENERATED_STAMP}
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E touch ${GENERATED_STAMP}
    BYPRODUCTS
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/logos_sdk.h"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/logos_sdk.cpp"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/package_manager_api.h"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/package_manager_api.cpp"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/template_module_api.h"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/template_module_api.cpp"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator for test_install_package"
    VERBATIM
)

add_custom_target(run_cpp_generator_test_install_package DEPENDS ${GENERATED_STAMP})
if (TARGET cpp_generator_build)
    add_dependencies(run_cpp_generator_test_install_package cpp_generator_build)
endif()

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Define the test_install_package application sources
set(TEST_INSTALL_PACKAGE_SOURCES
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.h
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp PROPERTIES GENERATED TRUE)

# Create the test_install_package application
add_executable(test_install_package ${TEST_INSTALL_PACKAGE_SOURCES})
add_dependencies(test_install_package run_cpp_generator_test_install_package)

# Link the test_install_package application with the logos core library and Qt RemoteObjects
target_link_libraries(test_install_package PRIVATE 
    ${LOGOS_CORE_LIBRARY} 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)

# Include directories for the test_install_package application
target_include_directories(test_install_package PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 
