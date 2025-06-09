#include <iostream>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>
#include "../../core/src/logos_core.h"

// Qt-style plugin testing utility class
class PluginTester {
public:
    // Get current loaded plugins as QStringList
    static QStringList getLoadedPlugins() {
        QStringList result;
        char** plugins = logos_core_get_loaded_plugins();
        
        if (plugins != nullptr) {
            int i = 0;
            while (plugins[i] != nullptr) {
                result << QString::fromUtf8(plugins[i]);
                delete[] plugins[i];
                i++;
            }
            delete[] plugins;
        }
        
        return result;
    }

    // Print loaded plugins in a nice format
    static void printLoadedPlugins(const QString& title = "Loaded plugins") {
        QStringList plugins = getLoadedPlugins();
        qDebug() << title << "(" << plugins.size() << "):" << plugins.join(", ");
    }

    // Generic assertion: check if loaded plugins exactly match expected plugins
    static bool assertEqual(const QStringList& expected, int errorCode = -1, 
                           const QString& message = "Plugin assertion") {
        QStringList loaded = getLoadedPlugins();
        QStringList sortedExpected = expected;
        QStringList sortedLoaded = loaded;

        sortedExpected.sort();
        sortedLoaded.sort();

        if (sortedExpected != sortedLoaded) {
            qCritical() << "FAIL:" << message;
            qCritical() << "  Expected:" << expected.join(", ");
            qCritical() << "  Actual:  " << loaded.join(", ");
            
            if (errorCode >= 0) {
                logos_core_cleanup();
                exit(errorCode);
            }
            return false;
        }

        qDebug() << "PASS:" << message << "- plugins:" << expected.join(", ");
        return true;
    }

    // Check if specific plugins are loaded (subset check)
    static bool assertContains(const QStringList& expectedPlugins, int errorCode = -1,
                              const QString& message = "Plugin contains assertion") {
        QStringList loaded = getLoadedPlugins();

        for (const QString& plugin : expectedPlugins) {
            if (!loaded.contains(plugin)) {
                qCritical() << "FAIL:" << message;
                qCritical() << "  Missing plugin:" << plugin;
                qCritical() << "  Loaded plugins:" << loaded.join(", ");

                if (errorCode >= 0) {
                    logos_core_cleanup();
                    exit(errorCode);
                }
                return false;
            }
        }

        qDebug() << "PASS:" << message << "- contains:" << expectedPlugins.join(", ");
        return true;
    }

    // Check if no plugins or specific plugins are NOT loaded
    static bool assertNotContains(const QStringList& unexpectedPlugins, int errorCode = -1,
                                 const QString& message = "Plugin exclusion assertion") {
        QStringList loaded = getLoadedPlugins();

        for (const QString& plugin : unexpectedPlugins) {
            if (loaded.contains(plugin)) {
                qCritical() << "FAIL:" << message;
                qCritical() << "  Unexpected plugin found:" << plugin;
                qCritical() << "  Loaded plugins:" << loaded.join(", ");

                if (errorCode >= 0) {
                    logos_core_cleanup();
                    exit(errorCode);
                }
                return false;
            }
        }

        qDebug() << "PASS:" << message << "- excludes:" << unexpectedPlugins.join(", ");
        return true;
    }

    // Check plugin count
    static bool assertCount(int expectedCount, int errorCode = -1,
                           const QString& message = "Plugin count assertion") {
        QStringList loaded = getLoadedPlugins();

        if (loaded.size() != expectedCount) {
            qCritical() << "FAIL:" << message;
            qCritical() << "  Expected count:" << expectedCount;
            qCritical() << "  Actual count:  " << loaded.size();
            qCritical() << "  Loaded plugins:" << loaded.join(", ");

            if (errorCode >= 0) {
                logos_core_cleanup();
                exit(errorCode);
            }
            return false;
        }

        qDebug() << "PASS:" << message << "- count:" << expectedCount;
        return true;
    }
};

int main(int argc, char *argv[])
{
    std::cout << "Test Simple Application Starting..." << std::endl;

    // Initialize the logos core library
    logos_core_init(argc, argv);

    std::cout << "Logos Core initialized successfully!" << std::endl;

    // Set the custom plugins directory
    QString pluginsDir = QDir::cleanPath(QDir::currentPath() + "/modules");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(pluginsDir.toUtf8().constData());

    // Start the logos core functionality
    logos_core_start();

    std::cout << "Logos Core started successfully!" << std::endl;

    // Show initial plugin state
    qDebug() << "\n=== Initial Plugin State ===";
    PluginTester::printLoadedPlugins("Initial plugins");
    
    // Assert that core_manager plugin is loaded initially
    PluginTester::assertEqual(QStringList{"core_manager"}, 1, "Initial state check");

    // Load the template_module plugin specifically
    qDebug() << "\n=== Loading template_module plugin ===";
    if (logos_core_load_plugin("template_module")) {
        qDebug() << "Successfully loaded template_module plugin";
    } else {
        qCritical() << "Failed to load template_module plugin";
        logos_core_cleanup();
        exit(2);
    }

    // Show and verify final plugin state
    qDebug() << "\n=== Final Plugin State ===";
    PluginTester::printLoadedPlugins("Final plugins");
    
    // Multiple assertion examples:
    PluginTester::assertEqual(QStringList{"core_manager", "template_module"}, 3, 
                             "Final state - exact match");

    // All assertions passed - test completed successfully
    std::cout << "\nAll plugin tests passed successfully!" << std::endl;
    std::cout << "Test Simple application completed." << std::endl;

    // Clean up resources
    logos_core_cleanup();

    return 0;
} 
