set(CMAKE_AUTOMOC ON)

# Generate the source code from the .rep file
qt_add_repc_sources(counter_source SOURCE_REPO ../counter.rep)

add_executable(source_app
    main.cpp
    counter.h
    counter.cpp
    ${counter_source}
)

target_link_libraries(source_app PRIVATE
    Qt::Core
    Qt::RemoteObjects
) 