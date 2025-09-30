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
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/chat_api.h"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/chat_api.cpp"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/waku_module_api.h"
        "${REPO_ROOT}/logos-cpp-sdk/cpp/generated/waku_module_api.cpp"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator for test_chat"
    VERBATIM
)

add_custom_target(run_cpp_generator_test_chat DEPENDS ${GENERATED_STAMP})
if (TARGET cpp_generator_build)
    add_dependencies(run_cpp_generator_test_chat cpp_generator_build)
endif()

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Define the test_chat application sources
set(TEST_CHAT_SOURCES
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/logos_api_consumer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/module_proxy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp
)

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated/logos_sdk.cpp PROPERTIES GENERATED TRUE)

# Create the test_chat application
add_executable(test_chat ${TEST_CHAT_SOURCES})
add_dependencies(test_chat run_cpp_generator_test_chat)

# Link the test_chat application with the logos core library and Qt RemoteObjects
target_link_libraries(test_chat PRIVATE 
    ${LOGOS_CORE_LIBRARY} 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)

# Include directories for the test_chat application
target_include_directories(test_chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-liblogos/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-liblogos
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp/generated
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 
