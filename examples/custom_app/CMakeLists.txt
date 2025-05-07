set(CMAKE_AUTOMOC ON)

# Define the custom application sources
set(CUSTOM_APP_SOURCES
    custom_app.cpp
)

# Create the custom application
add_executable(custom_app ${CUSTOM_APP_SOURCES})

# Link the custom application with the logos core library
target_link_libraries(custom_app PRIVATE ${LOGOS_CORE_LIBRARY} Qt${QT_VERSION_MAJOR}::Core)

# Include directories for the custom application
target_include_directories(custom_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 