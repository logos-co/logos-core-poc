set(CMAKE_AUTOMOC ON)

# Add SDK subdirectory to build the logos_sdk library
add_subdirectory(../../logos-cpp-sdk/cpp ${CMAKE_BINARY_DIR}/sdk)

# Define the library sources
set(LOGOS_CORE_SOURCES
    logos_core.cpp
    logos_core.h
    core_manager/core_manager.cpp
    core_manager/core_manager.h
    core_manager/core_manager_interface.h
    ../interface.h
)

# Define the host application sources
set(HOST_SOURCES
    main.cpp
)

# Define the logos host sources
set(LOGOS_HOST_SOURCES
    logos_host.cpp
    ../interface.h
)

# Create the logos core library
add_library(logos_core SHARED ${LOGOS_CORE_SOURCES})

# Set the LOGOS_CORE_LIBRARY definition for the library
target_compile_definitions(logos_core PRIVATE LOGOS_CORE_LIBRARY)

# Link Qt libraries and SDK to the library
target_link_libraries(logos_core PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    logos_sdk
)

# Include directories for the library
target_include_directories(logos_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
)

# Set RPATH settings for the library
if(APPLE)
    set_target_properties(logos_core PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE
        BUILD_WITH_INSTALL_RPATH TRUE)
else()
    set_target_properties(logos_core PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Create the host application
add_executable(logoscore ${HOST_SOURCES})

# Link the host application with the logos core library
target_link_libraries(logoscore PRIVATE logos_core)

# Include directories for the host application
target_include_directories(logoscore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
)

# Create the logos host application
add_executable(logos_host ${LOGOS_HOST_SOURCES})

# Link the logos host with Qt libraries
target_link_libraries(logos_host PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    logos_sdk
)

# Include directories for the logos host application
target_include_directories(logos_host PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../logos-cpp-sdk/cpp
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
)

# Set RPATH settings for the executable
if(APPLE)
    set_target_properties(logoscore PROPERTIES
        INSTALL_RPATH "@loader_path/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE)
else()
    set_target_properties(logoscore PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE)
endif()

# Set RPATH settings for the logos host executable
if(APPLE)
    set_target_properties(logos_host PROPERTIES
        INSTALL_RPATH "@loader_path/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE)
else()
    set_target_properties(logos_host PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE)
endif() 