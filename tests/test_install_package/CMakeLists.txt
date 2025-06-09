set(CMAKE_AUTOMOC ON)

# Find Qt RemoteObjects (needed for LogosAPI)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Define the test_install_package application sources
set(TEST_INSTALL_PACKAGE_SOURCES
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp/logos_api.h
)

# Create the test_install_package application
add_executable(test_install_package ${TEST_INSTALL_PACKAGE_SOURCES})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../../SDK/cpp
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 