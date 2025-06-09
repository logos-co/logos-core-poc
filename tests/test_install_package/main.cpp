#include <iostream>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>
#include <QVariantList>
#include <QFile>
#include "../../core/src/logos_core.h"
#include "../../SDK/cpp/logos_api.h"

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

// Simple event tracker
class EventTracker {
public:
    static bool eventReceived;
    static QVariantList lastEventData;
    
    static void onEvent(const QString& eventName, const QVariantList& data) {
        qDebug() << "Event received:" << eventName << "with data:" << data;
        eventReceived = true;
        lastEventData = data;
    }
};

bool EventTracker::eventReceived = false;
QVariantList EventTracker::lastEventData;

int main(int argc, char *argv[])
{
    std::cout << "Test Install Package Application Starting..." << std::endl;

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

    // Load the package_manager plugin
    qDebug() << "\n=== Loading package_manager plugin ===";
    if (logos_core_load_plugin("package_manager")) {
        qDebug() << "Successfully loaded package_manager plugin";
    } else {
        qCritical() << "Failed to load package_manager plugin";
        logos_core_cleanup();
        exit(1);
    }

    // Verify package_manager is loaded
    PluginTester::printLoadedPlugins("Plugins after loading package_manager");
    PluginTester::assertEqual(QStringList{"core_manager", "package_manager"}, 2, "package_manager should be loaded");

    // Check that packages directory exists and contains template_module
    QString packagesDir = QDir::cleanPath(QDir::currentPath() + "/packages");
    qDebug() << "\n=== Checking Packages Directory ===";
    qDebug() << "Packages directory:" << packagesDir;
    
    QDir packages(packagesDir);
    if (packages.exists()) {
        QStringList packageFiles = packages.entryList(QStringList() << "*.dylib" << "*.so" << "*.dll", QDir::Files);
        qDebug() << "Available packages:" << packageFiles;
        
        if (packageFiles.contains("template_module_plugin.dylib") || 
            packageFiles.contains("template_module_plugin.so") ||
            packageFiles.contains("template_module_plugin.dll")) {
            qDebug() << "✓ template_module package found and ready for installation";
        } else {
            qDebug() << "⚠ template_module package not found in packages directory";
        }
    } else {
        qDebug() << "⚠ Packages directory does not exist";
    }

        // Test package installation via package_manager
    qDebug() << "\n=== Testing Package Installation ===";
    
    // Initialize LogosAPI for testing
    LogosAPI testAPI("local:logoscore_registry");
    
    // Determine the correct file extension for this platform
    QString libExt;
#ifdef Q_OS_DARWIN
    libExt = "dylib";
#elif defined(Q_OS_LINUX)
    libExt = "so";
#else
    libExt = "dll"; // Windows
#endif
    
    // Build the file path to the template_module plugin in packages directory
    QString packageFileName = QString("template_module_plugin.%1").arg(libExt);
    QString filePath = QDir::cleanPath(QDir::currentPath() + "/packages/" + packageFileName);
    
    qDebug() << "Attempting to install plugin from:" << filePath;
    
    // Check if the package file exists
    if (!QFile::exists(filePath)) {
        qCritical() << "FAIL: Package file does not exist at:" << filePath;
        logos_core_cleanup();
        exit(1);
    }
    
    qDebug() << "✓ Package file found, proceeding with installation...";
    
    // Call package_manager's installPlugin method
    QVariant result = testAPI.callRemoteMethod("package_manager", "installPlugin", filePath);
    bool installSuccess = result.toBool();
    
    if (installSuccess) {
        qDebug() << "\n\n\n\nSUCCESS: Package installation completed successfully";
        
        // Check if the plugin is now loaded
        qDebug() << "\n=== Post-Installation Plugin State ===";
        PluginTester::printLoadedPlugins("Plugins after installation");
        
        // Verify that template_module is now loaded
        PluginTester::assertContains(QStringList{"template_module"}, 4, 
                                   "template_module should be loaded after installation");
        
        // Test that all three plugins are loaded: core_manager, package_manager, and template_module
        PluginTester::assertEqual(QStringList{"core_manager", "package_manager", "template_module"}, 5, 
                                 "All three plugins should be loaded after installation");
        
    } else {
        qCritical() << "FAIL: Package installation failed";
        qDebug() << "Installation result:" << result;
        logos_core_cleanup();
        exit(1);
    }

    // All assertions passed - test completed successfully
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "✓ Plugin loading tests passed";
    qDebug() << "✓ Package discovery tests passed";
    qDebug() << "✓ Package installation tests passed";
    qDebug() << "✓ Template module is now available and loaded";

    // Clean up resources
    logos_core_cleanup();

    return 0;
} 