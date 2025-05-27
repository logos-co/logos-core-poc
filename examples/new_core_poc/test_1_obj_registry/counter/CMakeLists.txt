cmake_minimum_required(VERSION 3.14)
project(CounterApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core RemoteObjects)

# Define the rep file
set(REP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/loggerinterface.rep)

# Create the counter executable
add_executable(process_host_counter process_host_counter.cpp)

# Add the rep file to the counter as replica
qt6_add_repc_replicas(process_host_counter ${REP_FILE})

# Link the counter with the Qt libraries
target_link_libraries(process_host_counter PRIVATE 
    Qt6::Core
    Qt6::RemoteObjects
)

# Include directories for the counter
target_include_directories(process_host_counter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Qt6_INCLUDE_DIRS}
) 