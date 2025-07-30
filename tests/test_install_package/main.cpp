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
#include "../../SDK/cpp/logos_api_client.h"

// Qt-style plugin testing utility class
class PluginTester {
public:
    // ANSI color codes for terminal output
    static const QString RED_BOLD;
    static const QString GREEN_BOLD;
    static const QString YELLOW_BOLD;
    static const QString RESET;
    
    // Print error message in red bold
    static void printError(const QString& message) {
        std::cerr << RED_BOLD.toStdString() << message.toStdString() << RESET.toStdString() << std::endl;
    }
    
    // Print success message in green bold
    static void printSuccess(const QString& message) {
        std::cout << GREEN_BOLD.toStdString() << message.toStdString() << RESET.toStdString() << std::endl;
    }
    
    // Print warning message in yellow bold
    static void printWarning(const QString& message) {
        std::cout << YELLOW_BOLD.toStdString() << message.toStdString() << RESET.toStdString() << std::endl;
    }
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
            printError(QString("FAIL: %1").arg(message));
            printError(QString("  Expected: %1").arg(expected.join(", ")));
            printError(QString("  Actual:   %1").arg(loaded.join(", ")));
            
            if (errorCode >= 0) {
                logos_core_cleanup();
                exit(errorCode);
            }
            return false;
        }

        printSuccess(QString("PASS: %1 - plugins: %2").arg(message).arg(expected.join(", ")));
        return true;
    }

    // Check if specific plugins are loaded (subset check)
    static bool assertContains(const QStringList& expectedPlugins, int errorCode = -1,
                              const QString& message = "Plugin contains assertion") {
        QStringList loaded = getLoadedPlugins();

        for (const QString& plugin : expectedPlugins) {
            if (!loaded.contains(plugin)) {
                printError(QString("FAIL: %1").arg(message));
                printError(QString("  Missing plugin: %1").arg(plugin));
                printError(QString("  Loaded plugins: %1").arg(loaded.join(", ")));

                if (errorCode >= 0) {
                    logos_core_cleanup();
                    exit(errorCode);
                }
                return false;
            }
        }

        printSuccess(QString("PASS: %1 - contains: %2").arg(message).arg(expectedPlugins.join(", ")));
        return true;
    }

    // Check if no plugins or specific plugins are NOT loaded
    static bool assertNotContains(const QStringList& unexpectedPlugins, int errorCode = -1,
                                 const QString& message = "Plugin exclusion assertion") {
        QStringList loaded = getLoadedPlugins();

        for (const QString& plugin : unexpectedPlugins) {
            if (loaded.contains(plugin)) {
                printError(QString("FAIL: %1").arg(message));
                printError(QString("  Unexpected plugin found: %1").arg(plugin));
                printError(QString("  Loaded plugins: %1").arg(loaded.join(", ")));

                if (errorCode >= 0) {
                    logos_core_cleanup();
                    exit(errorCode);
                }
                return false;
            }
        }

        printSuccess(QString("PASS: %1 - excludes: %2").arg(message).arg(unexpectedPlugins.join(", ")));
        return true;
    }

    // Check plugin count
    static bool assertCount(int expectedCount, int errorCode = -1,
                           const QString& message = "Plugin count assertion") {
        QStringList loaded = getLoadedPlugins();

        if (loaded.size() != expectedCount) {
            printError(QString("FAIL: %1").arg(message));
            printError(QString("  Expected count: %1").arg(expectedCount));
            printError(QString("  Actual count:   %1").arg(loaded.size()));
            printError(QString("  Loaded plugins: %1").arg(loaded.join(", ")));

            if (errorCode >= 0) {
                logos_core_cleanup();
                exit(errorCode);
            }
            return false;
        }

        printSuccess(QString("PASS: %1 - count: %2").arg(message).arg(expectedCount));
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

// Define static color constants
const QString PluginTester::RED_BOLD = "\033[1;31m";
const QString PluginTester::GREEN_BOLD = "\033[1;32m";
const QString PluginTester::YELLOW_BOLD = "\033[1;33m";
const QString PluginTester::RESET = "\033[0m";

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
    
    // Assert that core_manager and capability_module plugins are loaded initially
    // (capability_module is auto-loaded because it's in the modules directory)
    PluginTester::assertEqual(QStringList{"core_manager", "capability_module"}, 1, "Initial state check");

    // Load the package_manager plugin
    qDebug() << "\n=== Loading package_manager plugin ===";
    if (logos_core_load_plugin("package_manager")) {
        PluginTester::printSuccess("Successfully loaded package_manager plugin");
    } else {
        PluginTester::printError("CRITICAL: Failed to load package_manager plugin");
        logos_core_cleanup();
        exit(1);
    }

    // Verify package_manager is loaded
    PluginTester::printLoadedPlugins("Plugins after loading package_manager");
    PluginTester::assertEqual(QStringList{"core_manager", "capability_module", "package_manager"}, 2, "package_manager should be loaded");

    // Verify capability_module is already loaded (auto-loaded from modules directory)
    qDebug() << "\n=== Verifying capability_module is loaded ===";
    PluginTester::printLoadedPlugins("Current plugins state");
    PluginTester::assertContains(QStringList{"capability_module"}, 3, "capability_module should be loaded");

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
        LogosAPI testAPI("test_install_package");
    
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
        PluginTester::printError(QString("CRITICAL: Package file does not exist at: %1").arg(filePath));
        logos_core_cleanup();
        exit(1);
    }
    
    qDebug() << "✓ Package file found, proceeding with installation...";
    
    // Call package_manager's installPlugin method
    QVariant result = testAPI.getClient("package_manager")->invokeRemoteMethod("package_manager", "installPlugin", filePath);
    bool installSuccess = result.toBool();
    
    if (installSuccess) {
        qDebug() << "\n\n\n\nSUCCESS: Package installation completed successfully";

        // logos load template_module
        logos_core_load_plugin("template_module");
        
        // Check if the plugin is now loaded
        qDebug() << "\n=== Post-Installation Plugin State ===";
        PluginTester::printLoadedPlugins("Plugins after installation");
        
        // Verify that template_module is now loaded
        PluginTester::assertContains(QStringList{"template_module"}, 4, 
                                  "template_module should be loaded after installation");
        
        // Test that all four plugins are loaded: core_manager, package_manager, capability_module, and template_module
        PluginTester::assertEqual(QStringList{"core_manager", "package_manager", "capability_module", "template_module"}, 5, 
                                "All four plugins should be loaded after installation");
        
        // Test the event system
        qDebug() << "\n=== Testing Template Module Events ===";

        // Initialize LogosAPI for testing
        LogosAPI eventTestAPI("test_install_package");

        // Get template_module object for event listening
        QObject* templateModuleObj = eventTestAPI.getClient("template_module")->requestObject("template_module");
        if (!templateModuleObj) {
            PluginTester::printError("CRITICAL: Failed to get template_module from registry");
            logos_core_cleanup();
            exit(1);
        }
        
        // Register event listener
        EventTracker::eventReceived = false;
        eventTestAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "fooTriggered", EventTracker::onEvent);
        
        // Call foo method using remote API
        QVariant testParam = "hello_world";
        qDebug() << "Calling foo() remotely with parameter:" << testParam;
        
        QVariant eventResult = eventTestAPI.getClient("template_module")->invokeRemoteMethod("template_module", "foo", testParam);
        if (!eventResult.isValid() || !eventResult.toBool()) {
            PluginTester::printError("CRITICAL: Failed to call foo() method or method returned false");
            logos_core_cleanup();
            exit(1);
        }
        
        qDebug() << "foo() method called successfully";
        
        // Process events briefly
        QCoreApplication::processEvents();
        
        // Check if event was received
        if (EventTracker::eventReceived) {
            PluginTester::printSuccess("SUCCESS: Event received!");
            if (EventTracker::lastEventData.size() > 0 && 
                EventTracker::lastEventData[0].toString() == testParam.toString()) {
                PluginTester::printSuccess(QString("SUCCESS: Event contains correct parameter: %1").arg(testParam.toString()));
            } else {
                PluginTester::printError("CRITICAL: Event parameter mismatch");
                logos_core_cleanup();
                exit(1);
            }
        } else {
            PluginTester::printError("CRITICAL: No event received");
            logos_core_cleanup();
            exit(1);
        }
        
    } else {
        PluginTester::printError("CRITICAL: Package installation failed");
        PluginTester::printError(QString("Installation result: %1").arg(result.toString()));
        logos_core_cleanup();
        exit(1);
    }

    // All assertions passed - test completed successfully
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "✓ Plugin loading tests passed";
    qDebug() << "✓ Package discovery tests passed";
    qDebug() << "✓ Package installation tests passed";
    qDebug() << "✓ Template module is now available and loaded";
    qDebug() << "✓ Capability module is loaded and functional";
    qDebug() << "✓ Event system tests passed";
    qDebug() << "✓ Template module foo() event trigger test passed";

    // keep the app running until ctrl+c
    PluginTester::printSuccess("Application running - Press Ctrl+C to exit...");
    
    // Create a QCoreApplication instance for event loop
    QCoreApplication app(argc, argv);
    
    // Run the event loop until interrupted
    app.exec();

    // Clean up resources
    // logos_core_cleanup();

    return 0;
} 