cmake_minimum_required(VERSION 3.16)
project(LogosApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)  # Enable Qt's Meta-Object Compiler
set(CMAKE_AUTORCC ON)  # Enable automatic compilation of resource files
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# RPATH settings for macOS
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add interfaces directory to include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add logos_core include directories - use a variable for the core path
set(LOGOS_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../core)
include_directories(
    ${LOGOS_CORE_DIR}/host
    ${LOGOS_CORE_DIR}
)

# Set logos_core library path using the variable
link_directories(${LOGOS_CORE_DIR}/build/lib)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
    sidebarbutton.h
    sidebarbutton.cpp
    mdichild.h
    mdichild.cpp
    mdiview.h
    mdiview.cpp
    modulesview.h
    modulesview.cpp
    dashboardview.h
    dashboardview.cpp
    coremoduleview.h
    coremoduleview.cpp
    interfaces/IComponent.h
    resources.qrc
)

qt_add_executable(LogosApp
    ${PROJECT_SOURCES}
)

# Link with logos_core library using a more flexible approach
target_link_libraries(LogosApp PRIVATE 
    Qt6::Widgets
    logos_core
)

# Copy the core library to the app's lib directory and fix its install name
add_custom_command(TARGET LogosApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E copy ${LOGOS_CORE_DIR}/build/lib/liblogos_core.dylib ${CMAKE_CURRENT_BINARY_DIR}/lib/
    COMMAND install_name_tool -id @rpath/liblogos_core.dylib ${CMAKE_CURRENT_BINARY_DIR}/lib/liblogos_core.dylib
    # Add additional rpath entries for flexibility when moving the binary
    COMMAND install_name_tool -add_rpath "@executable_path/lib" ${CMAKE_CURRENT_BINARY_DIR}/LogosApp
    COMMAND install_name_tool -add_rpath "@loader_path/lib" ${CMAKE_CURRENT_BINARY_DIR}/LogosApp
    COMMENT "Copying logos_core library and fixing installation paths"
)

# Create bin/plugins directory in build folder
add_custom_command(TARGET LogosApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/bin/plugins
    COMMAND ${CMAKE_COMMAND} -E echo "Creating plugins directory at ${CMAKE_CURRENT_BINARY_DIR}/bin/plugins"
)

# Build the plugins first
add_custom_command(TARGET LogosApp PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Building plugins..."
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/../plugins && mkdir -p build && cd build && cmake .. && make
)
