#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include <QString>
#include <QStringList>
#include <memory>

// Forward declarations to avoid exposing logos_core implementation details
struct PluginsArrayDeleter {
    void operator()(char** plugins);
};

class CoreManager {
public:
    static CoreManager& instance();
    
    // Initialize logos_core
    void initialize(int argc, char* argv[]);
    
    // Set plugins directory
    void setPluginsDirectory(const QString& directory);
    
    // Start logos_core
    void start();
    
    // Clean up logos_core
    void cleanup();
    
    // Get list of loaded plugins
    QStringList getLoadedPlugins();
    
    // Delete these methods to ensure singleton pattern
    CoreManager(const CoreManager&) = delete;
    void operator=(const CoreManager&) = delete;

    bool loadPlugin(const QString& pluginName);

private:
    // Private constructor for singleton pattern
    CoreManager() = default;
    
    // Helper method to convert C-style array to QStringList
    QStringList convertPluginsToStringList(char** plugins);
};

#endif // CORE_MANAGER_H 