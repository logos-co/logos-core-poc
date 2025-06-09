set(CMAKE_AUTOMOC ON)

# Define the test_simple application sources
set(TEST_SIMPLE_SOURCES
    main.cpp
)

# Create the test_simple application
add_executable(test_simple ${TEST_SIMPLE_SOURCES})

# Link the test_simple application with the logos core library
target_link_libraries(test_simple PRIVATE ${LOGOS_CORE_LIBRARY} Qt${QT_VERSION_MAJOR}::Core)

# Include directories for the test_simple application
target_include_directories(test_simple PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../core
    ${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS}
) 