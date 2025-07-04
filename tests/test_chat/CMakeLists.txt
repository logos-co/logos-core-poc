set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Define the test_chat application sources
set(TEST_CHAT_SOURCES
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/module_proxy.h
)

# Create the test_chat application
add_executable(test_chat ${TEST_CHAT_SOURCES})

# Link the test_chat application with the logos core library and Qt RemoteObjects
target_link_libraries(test_chat PRIVATE 
    ${LOGOS_CORE_LIBRARY} 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
)

# Include directories for the test_chat application
target_include_directories(test_chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 