set(CMAKE_AUTOMOC ON)

add_executable(registry_app
    main.cpp
)

target_link_libraries(registry_app PRIVATE
    Qt::Core
    Qt::RemoteObjects
) 