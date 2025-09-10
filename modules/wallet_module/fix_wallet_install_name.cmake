# CMake script to fix wallet library install name on macOS
# Use passed paths or fall back to defaults
if(NOT WALLET_LIB_PATH)
    set(WALLET_LIB_PATH "${CMAKE_BINARY_DIR}/modules/libgowalletsdk.dylib")
endif()
if(NOT PLUGIN_PATH)
    set(PLUGIN_PATH "${CMAKE_BINARY_DIR}/modules/wallet_module_plugin.dylib")
endif()

message(STATUS "Looking for wallet library at: ${WALLET_LIB_PATH}")
message(STATUS "Looking for plugin at: ${PLUGIN_PATH}")

if(EXISTS "${WALLET_LIB_PATH}")
    message(STATUS "Found wallet library, updating install name...")
    # Fix the install name of the wallet library itself
    execute_process(
        COMMAND install_name_tool -id "@rpath/libgowalletsdk.dylib" "${WALLET_LIB_PATH}"
        RESULT_VARIABLE result1
    )
    if(result1)
        message(WARNING "Failed to update install name for libgowalletsdk.dylib")
    else()
        message(STATUS "Successfully updated install name for libgowalletsdk.dylib")
    endif()
    
    # Fix the reference in the wallet plugin to use @rpath
    execute_process(
        COMMAND install_name_tool -change "libgowalletsdk.dylib" "@rpath/libgowalletsdk.dylib" "${PLUGIN_PATH}"
        RESULT_VARIABLE result2
    )
    if(result2)
        message(WARNING "Failed to update reference to libgowalletsdk.dylib in wallet plugin")
    else()
        message(STATUS "Successfully updated reference to libgowalletsdk.dylib in wallet plugin")
    endif()
else()
    message(STATUS "Wallet library not found at ${WALLET_LIB_PATH}, skipping install name update")
    # Check if plugin exists and try to fix its reference anyway
    if(EXISTS "${PLUGIN_PATH}")
        message(STATUS "Attempting to fix plugin reference without library...")
        execute_process(
            COMMAND install_name_tool -change "libgowalletsdk.dylib" "@rpath/libgowalletsdk.dylib" "${PLUGIN_PATH}"
            RESULT_VARIABLE result3
        )
        if(result3)
            message(STATUS "Could not update plugin reference (library may not be linked)")
        else()
            message(STATUS "Updated plugin reference to use @rpath")
        endif()
    endif()
endif()
