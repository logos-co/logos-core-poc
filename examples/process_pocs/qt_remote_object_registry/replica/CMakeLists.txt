set(CMAKE_AUTOMOC ON)

# Generate the replica code from the .rep file
qt_add_repc_replicas(counter_replica ./counter.rep)

add_executable(replica_app
    main.cpp
    ${counter_replica}
)

target_link_libraries(replica_app PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::RemoteObjects
) 