cmake_minimum_required(VERSION 3.10)
project(chat)

# Enable C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core RemoteObjects REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core RemoteObjects REQUIRED)
endif()

# Run Logos C++ generator on metadata before compilation
set(CPP_GENERATOR "${CMAKE_SOURCE_DIR}/../build/cpp-generator/bin/logos-cpp-generator")
set(REPO_ROOT "${CMAKE_SOURCE_DIR}/..")
set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
add_custom_target(run_cpp_generator_chat
    COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --module-dir "${PLUGINS_OUTPUT_DIR}"
    WORKING_DIRECTORY "${REPO_ROOT}"
    COMMENT "Running logos-cpp-generator on ${METADATA_JSON} with module-dir ${PLUGINS_OUTPUT_DIR}"
    VERBATIM
)
# Ensure generator binary exists before running
add_dependencies(run_cpp_generator_chat cpp_generator_build)

# (deps generator added after metadata parsing below)

# Set automoc on
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Protocol Buffers
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(absl QUIET)

# Generate protobuf files
set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf/message.proto")
set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/message.pb.cc")
set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/message.pb.h")

add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/src/protobuf ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Running C++ protocol buffer compiler on message.proto"
    VERBATIM
)

# Create custom target for protocol buffers
add_custom_target(generate_protos DEPENDS ${PROTO_SRC} ${PROTO_HDR})

# Add the library
add_library(chat SHARED
    chat_plugin.cpp
    chat_plugin.h
    chat_interface.h
    src/chat_api.cpp
    ${PROTO_SRC}
    ${PROTO_HDR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/generated/logos_sdk.cpp
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

set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/generated/logos_sdk.cpp PROPERTIES GENERATED TRUE)

# Make sure protocol buffers are generated before building
add_dependencies(chat generate_protos)
add_dependencies(chat run_cpp_generator_chat)

# Set output name without lib prefix and with _plugin postfix
set_target_properties(chat PROPERTIES
    PREFIX ""
    OUTPUT_NAME "chat_plugin")

# No extra dependency printing; generator handles dependencies via --module-dir

# Include directories
target_include_directories(chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Protobuf_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/generated
)

# Link against libraries
target_link_libraries(chat PRIVATE 
    Qt::Core
    Qt::RemoteObjects
    ${Protobuf_LIBRARIES}
    Threads::Threads
)

# Link against Abseil libraries if found
if(absl_FOUND)
    target_link_libraries(chat PRIVATE
        absl::base
        absl::strings
        absl::log
        absl::check
    )
endif()

# Set common properties for both platforms
set_target_properties(chat PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(chat PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)
else()
    # Linux specific settings
    set_target_properties(chat PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

# Print status messages
message(STATUS "Protobuf version: ${Protobuf_VERSION}")
message(STATUS "Protobuf include dirs: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libraries: ${Protobuf_LIBRARIES}")
message(STATUS "Chat Plugin configured successfully") 
