#include <iostream>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDebug>
#include <QVariantList>
#include "../../core/src/logos_core.h"
#include "../../logos-cpp-sdk/cpp/logos_api.h"
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"
#include "../../logos-cpp-sdk/cpp/generated/logos_sdk.h"
#include <QTimer>
#include <QDateTime>

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
    std::cout << "Test Chat Application Starting..." << std::endl;

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

    // Load the waku plugin specifically
    qDebug() << "\n=== Loading waku plugin ===";
    if (logos_core_load_plugin("waku_module")) {
        PluginTester::printSuccess("Successfully loaded waku plugin");
    } else {
        PluginTester::printError("CRITICAL: Failed to load waku plugin");
        logos_core_cleanup();
        exit(1);
    }

    // Load the chat plugin specifically
    qDebug() << "\n=== Loading chat plugin ===";
    if (logos_core_load_plugin("chat")) {
        PluginTester::printSuccess("Successfully loaded chat plugin");
    } else {
        PluginTester::printError("CRITICAL: Failed to load chat plugin");
        logos_core_cleanup();
        exit(1);
    }

    // Show and verify final plugin state
    qDebug() << "\n=== Final Plugin State ===";
    PluginTester::printLoadedPlugins("Final plugins");

    // Multiple assertion examples:
    PluginTester::assertEqual(QStringList{"core_manager", "chat", "waku_module"}, 3, 
                             "Final state - exact match");

    // Test the chat and waku modules
    qDebug() << "\n=== Testing Chat and Waku Modules ===";

            // Initialize LogosAPI and generated wrappers
        LogosAPI* logosAPI = new LogosAPI("core");
        LogosModules* logos = new LogosModules(logosAPI);

    // Get waku object for testing
    QObject* wakuObj = logosAPI->getClient("waku_module")->requestObject("waku_module");
    if (!wakuObj) {
        PluginTester::printWarning("Waku module object not accessible via registry (this may be expected)");
    } else {
        PluginTester::printSuccess("Successfully got waku object from registry");
    }
    // Get chat object for testing
    QObject* chatObj = logosAPI->getClient("chat")->requestObject("chat");
    if (!chatObj) {
        PluginTester::printWarning("Chat module object not accessible via registry (this may be expected)");
    } else {
        PluginTester::printSuccess("Successfully got chat object from registry");
    }

    // Add onEvent handler for chatMessage
    qDebug() << "Setting up chatMessage event handler...";
    logos->chat.on("chatMessage", [](const QString& eventName, const QVariantList& data) {
        std::cout << "\n\n\n=== CHAT MESSAGE EVENT RECEIVED ===" << std::endl;
        std::cout << "Event: " << eventName.toStdString() << std::endl;
        if (data.size() >= 3) {
            std::cout << "Timestamp: " << data[0].toString().toStdString() << std::endl;
            std::cout << "Nick: " << data[1].toString().toStdString() << std::endl;
            std::cout << "Message: " << data[2].toString().toStdString() << std::endl;
        } else {
            std::cout << "Data: ";
            for (const QVariant& item : data) {
                std::cout << item.toString().toStdString() << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "===================================\n\n\n" << std::endl;
    });
    PluginTester::printSuccess("Chat message event handler registered");

    // Call initialize method on chat module
    qDebug() << "Calling chat initialize() method...";
    bool initSuccess = logos->chat.initialize();
    if (initSuccess) {
        PluginTester::printSuccess("Successfully called chat initialize() method");
        qDebug() << "Initialize result:" << initSuccess;
    } else {
        PluginTester::printWarning("Chat initialize() method call failed");
        exit(1);
    }

    // Join the "baixa-chiado" channel
    qDebug() << "Calling chat joinChannel() method with channel 'baixa-chiado'...";
    bool joinSuccess = logos->chat.joinChannel(QString("baixa-chiado"));
    if (joinSuccess) {
        PluginTester::printSuccess("Successfully called chat joinChannel() method");
        qDebug() << "Join channel result:" << joinSuccess;
    } else {
        PluginTester::printWarning("Chat joinChannel() method call failed");
        exit(1);
    }

    // // Create a QTimer to periodically send test messages
    // qDebug() << "Setting up periodic message timer...";
    // QTimer* timer = new QTimer();
    // QObject::connect(timer, &QTimer::timeout, [&chatAPI]() {
    //     QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    //     QString testMessage = QString("Test message at %1").arg(timestamp);
        
    //     qDebug() << "Sending periodic test message:" << testMessage;
    //     QVariant sendResult = chatAPI.getClient("chat")->invokeRemoteMethod("chat", "sendMessage", 
    //                                                   QString("baixa-chiado"), 
    //                                                   QString("TestUser"), 
    //                                                   testMessage);
    //     if (sendResult.isValid()) {
    //         qDebug() << "Message sent successfully, result:" << sendResult;
    //     } else {
    //         qDebug() << "Failed to send message";
    //     }
    // });

    // // Start timer to fire every 10 seconds
    // timer->start(1000);
    // PluginTester::printSuccess("Periodic message timer started (sending every 10 seconds)");

    // listen to message history event historyMessage
    logos->chat.on("historyMessage", [](const QString& eventName, const QVariantList& data) {
        std::cout << "\n\n\n=== HISTORY MESSAGE EVENT RECEIVED ===" << std::endl;
        std::cout << "Event: " << eventName.toStdString() << std::endl;
        std::cout << "Data: " << data.first().toString().toStdString() << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        // print data
        for (const QVariant& item : data) {
            std::cout << item.toString().toStdString() << " ";
        }
        std::cout << std::endl;
        std::cout << "===================================\n\n\n" << std::endl;
        // exit(1);
    });
    PluginTester::printSuccess("History message event handler registered");

    // Call retrieveHistory method on chat module
    qDebug() << "Calling chat retrieveHistory() method...";
    bool retrieveSuccess = logos->chat.retrieveHistory(QString("baixa-chiado"));
    if (retrieveSuccess) {
        PluginTester::printSuccess("Successfully called chat retrieveHistory() method");
        qDebug() << "Retrieve history result:" << retrieveSuccess;
        // exit(1);
    } else {
        PluginTester::printWarning("Chat retrieveHistory() method call failed");
        exit(1);
    }

    // Note: Unlike template_module, chat and waku modules may not have 
    // simple test methods exposed. This is just verifying they load properly.

    // All assertions passed - test completed successfully
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "✓ Plugin loading tests passed";
    qDebug() << "✓ Chat module loading test passed";
    qDebug() << "✓ Waku module loading test passed";

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
