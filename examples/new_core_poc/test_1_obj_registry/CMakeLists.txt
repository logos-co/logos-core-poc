cmake_minimum_required(VERSION 3.14)
project(HelloWorld LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

# Define the rep file
set(REP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/loggerinterface.rep)

# Create the core application
add_executable(hello_world main.cpp)

# Add the rep file to the main application as source
qt6_add_repc_sources(hello_world ${REP_FILE})

# Link the core application with the Qt libraries
target_link_libraries(hello_world PRIVATE 
    Qt6::Core
    Qt6::RemoteObjects
)

# Include directories for the core application
target_include_directories(hello_world PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt6_INCLUDE_DIRS}
)

# Create the C++ process host executable with counter
# add_executable(process_host_counter process_host_counter.cpp)

# Add the rep file to the process host as replica
# qt6_add_repc_replicas(process_host_counter ${REP_FILE})

# Link the process host with the Qt libraries
# target_link_libraries(process_host_counter PRIVATE 
#     Qt6::Core
#     Qt6::RemoteObjects
# )

# # Include directories for the process host
# target_include_directories(process_host_counter PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_BINARY_DIR}
#     ${Qt6_INCLUDE_DIRS}
# ) 