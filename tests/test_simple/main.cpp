#include <iostream>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>
#include <QVariantList>
#include <QJsonArray>
#include <QJsonObject>
#include "../../core/src/logos_core.h"
#include "../../SDK/cpp/logos_api.h"

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
        PluginTester::printSuccess("Successfully loaded template_module plugin");
    } else {
        PluginTester::printError("CRITICAL: Failed to load template_module plugin");
        logos_core_cleanup();
        exit(1);
    }

    // Show and verify final plugin state
    qDebug() << "\n=== Final Plugin State ===";
    PluginTester::printLoadedPlugins("Final plugins");
    
    // Multiple assertion examples:
    PluginTester::assertEqual(QStringList{"core_manager", "template_module"}, 3, 
                             "Final state - exact match");

    // Test the event system
    qDebug() << "\n=== Testing Template Module Events ===";

    // Initialize LogosAPI for testing
    // LogosAPI testAPI("core_registry");
    LogosAPI testAPI("template_module");

    // Get template_module object for event listening
    QObject* templateModuleObj = testAPI.requestObject("template_module");
    if (!templateModuleObj) {
        PluginTester::printError("CRITICAL: Failed to get template_module from registry");
        logos_core_cleanup();
        exit(1);
    }
    
    // Register event listener
    EventTracker::eventReceived = false;
    testAPI.onEvent(templateModuleObj, nullptr, "fooTriggered", EventTracker::onEvent);
    
    // Call foo method using remote API
    QVariant testParam = "hello_world";
    qDebug() << "Calling foo() remotely with parameter:" << testParam;
    
    QVariant result = testAPI.invokeRemoteMethod("template_module", "foo", testParam);
    if (!result.isValid() || !result.toBool()) {
        PluginTester::printError("CRITICAL: Failed to call foo() method or method returned false");
        logos_core_cleanup();
        exit(1);
    }
    
    qDebug() << "foo() method called successfully";
    
    // Test the new bar method (void return type)
    qDebug() << "\n=== Testing Template Module bar() Method ===";
    QString barMessage = "Hello from bar method test!";
    qDebug() << "Calling bar() remotely with message:" << barMessage;
    
    QVariant barResult = testAPI.invokeRemoteMethod("template_module", "bar", barMessage);
    // For void methods, we expect the result to be valid (true) indicating successful call
    if (barResult.isValid()) {
        qDebug() << "bar() method called successfully";
        PluginTester::printSuccess("SUCCESS: bar() method called successfully");
    } else {
        PluginTester::printError("CRITICAL: Failed to call bar() method");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test the stringToBool method
    qDebug() << "\n=== Testing Template Module stringToBool() Method ===";
    
    // Test with "true"
    qDebug() << "Testing stringToBool with 'true'";
    QVariant trueResult = testAPI.invokeRemoteMethod("template_module", "stringToBool", QString("true"));
    if (trueResult.isValid() && trueResult.toBool() == true) {
        PluginTester::printSuccess("SUCCESS: stringToBool('true') returned true");
    } else {
        PluginTester::printError("CRITICAL: stringToBool('true') failed or returned wrong value");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with "false"
    qDebug() << "Testing stringToBool with 'false'";
    QVariant falseResult = testAPI.invokeRemoteMethod("template_module", "stringToBool", QString("false"));
    if (falseResult.isValid() && falseResult.toBool() == false) {
        PluginTester::printSuccess("SUCCESS: stringToBool('false') returned false");
    } else {
        PluginTester::printError("CRITICAL: stringToBool('false') failed or returned wrong value");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with "TRUE" (case insensitive)
    qDebug() << "Testing stringToBool with 'TRUE' (case insensitive)";
    QVariant trueCaseResult = testAPI.invokeRemoteMethod("template_module", "stringToBool", QString("TRUE"));
    if (trueCaseResult.isValid() && trueCaseResult.toBool() == true) {
        PluginTester::printSuccess("SUCCESS: stringToBool('TRUE') returned true (case insensitive)");
    } else {
        PluginTester::printError("CRITICAL: stringToBool('TRUE') failed or returned wrong value");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with invalid input
    qDebug() << "Testing stringToBool with 'invalid' (should default to false)";
    QVariant invalidResult = testAPI.invokeRemoteMethod("template_module", "stringToBool", QString("invalid"));
    if (invalidResult.isValid() && invalidResult.toBool() == false) {
        PluginTester::printSuccess("SUCCESS: stringToBool('invalid') correctly defaulted to false");
    } else {
        PluginTester::printError("CRITICAL: stringToBool('invalid') failed or returned wrong value");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test the getJsonArray method
    qDebug() << "\n=== Testing Template Module getJsonArray() Method ===";
    
    // Test with "numbers"
    qDebug() << "Testing getJsonArray with 'numbers'";
    QVariant numbersResult = testAPI.invokeRemoteMethod("template_module", "getJsonArray", QString("numbers"));
    if (numbersResult.isValid() && numbersResult.canConvert<QJsonArray>()) {
        QJsonArray numbersArray = numbersResult.toJsonArray();
        if (numbersArray.size() == 5 && numbersArray[0].toInt() == 1 && numbersArray[3].toInt() == 42) {
            PluginTester::printSuccess(QString("SUCCESS: getJsonArray('numbers') returned array with %1 elements").arg(numbersArray.size()));
        } else {
            PluginTester::printError("CRITICAL: getJsonArray('numbers') returned unexpected content");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: getJsonArray('numbers') failed or returned invalid result");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with "strings"
    qDebug() << "Testing getJsonArray with 'strings'";
    QVariant stringsResult = testAPI.invokeRemoteMethod("template_module", "getJsonArray", QString("strings"));
    if (stringsResult.isValid() && stringsResult.canConvert<QJsonArray>()) {
        QJsonArray stringsArray = stringsResult.toJsonArray();
        if (stringsArray.size() == 4 && stringsArray[0].toString() == "hello" && stringsArray[1].toString() == "world") {
            PluginTester::printSuccess(QString("SUCCESS: getJsonArray('strings') returned array with %1 elements").arg(stringsArray.size()));
        } else {
            PluginTester::printError("CRITICAL: getJsonArray('strings') returned unexpected content");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: getJsonArray('strings') failed or returned invalid result");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with "objects"
    qDebug() << "Testing getJsonArray with 'objects'";
    QVariant objectsResult = testAPI.invokeRemoteMethod("template_module", "getJsonArray", QString("objects"));
    if (objectsResult.isValid() && objectsResult.canConvert<QJsonArray>()) {
        QJsonArray objectsArray = objectsResult.toJsonArray();
        if (objectsArray.size() == 3) {
            QJsonObject firstPerson = objectsArray[0].toObject();
            if (firstPerson["name"].toString() == "Alice" && firstPerson["age"].toInt() == 30) {
                PluginTester::printSuccess(QString("SUCCESS: getJsonArray('objects') returned array with %1 person objects").arg(objectsArray.size()));
            } else {
                PluginTester::printError("CRITICAL: getJsonArray('objects') returned unexpected object content");
                logos_core_cleanup();
                exit(1);
            }
        } else {
            PluginTester::printError("CRITICAL: getJsonArray('objects') returned unexpected array size");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: getJsonArray('objects') failed or returned invalid result");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test with empty result
    qDebug() << "Testing getJsonArray with 'unknown' (should return empty array)";
    QVariant emptyResult = testAPI.invokeRemoteMethod("template_module", "getJsonArray", QString("unknown"));
    if (emptyResult.isValid() && emptyResult.canConvert<QJsonArray>()) {
        QJsonArray emptyArray = emptyResult.toJsonArray();
        if (emptyArray.size() == 0) {
            PluginTester::printSuccess("SUCCESS: getJsonArray('unknown') correctly returned empty array");
        } else {
            PluginTester::printError("CRITICAL: getJsonArray('unknown') should return empty array");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: getJsonArray('unknown') failed or returned invalid result");
        logos_core_cleanup();
        exit(1);
    }
    
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

    // All assertions passed - test completed successfully
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "✓ Plugin loading tests passed";
    qDebug() << "✓ Event system tests passed";
    qDebug() << "✓ Template module foo() event trigger test passed";
    qDebug() << "✓ Template module bar() void method test passed";
    qDebug() << "✓ Template module stringToBool() conversion test passed";
    qDebug() << "✓ Template module getJsonArray() return test passed";

    // // Clean up resources
    // logos_core_cleanup();
    
    // keep the app running until ctrl+c
    PluginTester::printSuccess("Application running - Press Ctrl+C to exit...");
    
    // Create a QCoreApplication instance for event loop
    QCoreApplication app(argc, argv);
    
    // Run the event loop until interrupted
    app.exec();
 
    // return 0;
}
