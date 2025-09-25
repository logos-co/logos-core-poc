cmake_minimum_required(VERSION 3.14)
project(LogosCppGenerator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Find Qt Core (QCoreApplication, QPluginLoader, QJson*)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(logos-cpp-generator
    main.cpp
)

target_link_libraries(logos-cpp-generator PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(logos-cpp-generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Output directories
set_target_properties(logos-cpp-generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


