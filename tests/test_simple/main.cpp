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
    
    // Assert that core_manager and capability_module plugins are loaded initially
    PluginTester::assertEqual(QStringList{"core_manager", "capability_module"}, 1, "Initial state check");

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
    PluginTester::assertEqual(QStringList{"core_manager", "template_module", "capability_module"}, 3, 
                             "Final state - exact match");

    // Test the event system
    qDebug() << "\n=== Testing Template Module Events ===";

            // Initialize LogosAPI for testing
        // LogosAPI testAPI("test_simple");
        LogosAPI testAPI("test_simple");

    // Get template_module object for event listening
    QObject* templateModuleObj = testAPI.getClient("template_module")->requestObject("template_module");
    if (!templateModuleObj) {
        PluginTester::printError("CRITICAL: Failed to get template_module from registry");
        logos_core_cleanup();
        exit(1);
    }
    
    // Register event listener
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "fooTriggered", EventTracker::onEvent);
    
    // Call foo method using remote API
    QVariant testParam = "hello_world";
    qDebug() << "Calling foo() remotely with parameter:" << testParam;
    
    QVariant result = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "foo", testParam);
    if (!result.isValid() || !result.toBool()) {
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

    // Test the new bar() method
    qDebug() << "\n=== Testing bar() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "barTriggered", EventTracker::onEvent);
    
    QString barMessage = "Hello from bar method!";
    qDebug() << "Calling bar() remotely with message:" << barMessage;
    
    QVariant barResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "bar", barMessage);
    if (!barResult.isValid()) {
        PluginTester::printError("CRITICAL: Failed to call bar() method");
        logos_core_cleanup();
        exit(1);
    }
    
    qDebug() << "bar() method called successfully";
    QCoreApplication::processEvents();
    
    if (EventTracker::eventReceived) {
        PluginTester::printSuccess("SUCCESS: bar() event received!");
        if (EventTracker::lastEventData.size() > 0 && 
            EventTracker::lastEventData[0].toString() == barMessage) {
            PluginTester::printSuccess(QString("SUCCESS: bar() event contains correct message: %1").arg(barMessage));
        } else {
            PluginTester::printError("CRITICAL: bar() event parameter mismatch");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: No bar() event received");
        logos_core_cleanup();
        exit(1);
    }

    qDebug() << "\n\n\n\n\n=== Testing Token Manager ===";
    qDebug() << "==========================================================================================";
    qDebug() << "==========================================================================================";
    // initialize for TemplateModule
    LogosAPI templateModuleAPI("test_simple");

    QString authToken = "test_auth_token_123";
    QString moduleName = "test_module";
    QString moduleToken = "test_module_token_456";

    qDebug() << "\n\n--------> Calling informModuleToken() with authToken:" << authToken << "moduleName:" << moduleName << "moduleToken:" << moduleToken;
    bool tokenResult = templateModuleAPI.getClient("capability_module")->informModuleToken(authToken, moduleName, moduleToken);
    if (!tokenResult) {
        PluginTester::printError("CRITICAL: Failed to call informModuleToken() method");
        logos_core_cleanup();
        exit(1);
    }

    // now call foo on template_module
    qDebug() << "\n\n--------> Calling foo() on template_module";
    QVariant fooResult = templateModuleAPI.getClient("template_module")->invokeRemoteMethod("template_module", "foo", "hello_world");
    if (!fooResult.isValid() || !fooResult.toBool()) {
        PluginTester::printError("CRITICAL: Failed to call foo() method");

        logos_core_cleanup();
        exit(1);
    }

    // Test the stringToBool() method
    qDebug() << "\n=== Testing stringToBool() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "stringToBoolTriggered", EventTracker::onEvent);
    
    // Test true values
    QStringList trueValues = {"true", "TRUE", "True", "1", "yes", "YES", "on", "ON"};
    for (const QString& value : trueValues) {
        qDebug() << "Testing stringToBool() with:" << value;
        QVariant boolResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "stringToBool", value);
        if (!boolResult.isValid() || !boolResult.toBool()) {
            PluginTester::printError(QString("CRITICAL: stringToBool() failed for true value: %1").arg(value));
            logos_core_cleanup();
            exit(1);
        }
        PluginTester::printSuccess(QString("PASS: stringToBool('%1') = true").arg(value));
    }
    
    // Test false values
    QStringList falseValues = {"false", "FALSE", "False", "0", "no", "NO", "off", "OFF", "random", ""};
    for (const QString& value : falseValues) {
        qDebug() << "Testing stringToBool() with:" << value;
        QVariant boolResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "stringToBool", value);
        if (!boolResult.isValid() || boolResult.toBool()) {
            PluginTester::printError(QString("CRITICAL: stringToBool() failed for false value: %1").arg(value));
            logos_core_cleanup();
            exit(1);
        }
        PluginTester::printSuccess(QString("PASS: stringToBool('%1') = false").arg(value));
    }

    // Test the getJsonArray() method
    qDebug() << "\n=== Testing getJsonArray() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "getJsonArrayTriggered", EventTracker::onEvent);
    
    // Test different array types
    QStringList arrayTypes = {"numbers", "strings", "mixed", "objects", "unknown"};
    QList<int> expectedSizes = {5, 4, 4, 2, 0}; // Expected array sizes for each type
    
    for (int i = 0; i < arrayTypes.size(); i++) {
        const QString& arrayType = arrayTypes[i];
        int expectedSize = expectedSizes[i];
        
        qDebug() << "Testing getJsonArray() with arrayType:" << arrayType;
        QVariant arrayResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getJsonArray", arrayType);
        
        if (!arrayResult.isValid()) {
            PluginTester::printError(QString("CRITICAL: getJsonArray() failed for type: %1").arg(arrayType));
            logos_core_cleanup();
            exit(1);
        }
        
        // Convert QVariant to QJsonArray
        QJsonArray jsonArray = arrayResult.toJsonArray();
        if (jsonArray.size() != expectedSize) {
            PluginTester::printError(QString("CRITICAL: getJsonArray('%1') size mismatch. Expected: %2, Got: %3")
                                   .arg(arrayType).arg(expectedSize).arg(jsonArray.size()));
            logos_core_cleanup();
            exit(1);
        }
        
        PluginTester::printSuccess(QString("PASS: getJsonArray('%1') returned array of size %2")
                                 .arg(arrayType).arg(jsonArray.size()));
        
        // Print array contents for verification
        qDebug() << "Array contents:" << jsonArray;
    }

    // Test specific array content validation
    qDebug() << "\n=== Validating specific array contents ===";
    
    // Test numbers array
    QVariant numbersResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getJsonArray", "numbers");
    QJsonArray numbersArray = numbersResult.toJsonArray();
    for (int i = 0; i < 5; i++) {
        if (numbersArray[i].toInt() != i + 1) {
            PluginTester::printError(QString("CRITICAL: Numbers array validation failed at index %1").arg(i));
            logos_core_cleanup();
            exit(1);
        }
    }
    PluginTester::printSuccess("PASS: Numbers array content validation");
    
    // Test strings array
    QVariant stringsResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getJsonArray", "strings");
    QJsonArray stringsArray = stringsResult.toJsonArray();
    QStringList expectedStrings = {"apple", "banana", "cherry", "date"};
    for (int i = 0; i < expectedStrings.size(); i++) {
        if (stringsArray[i].toString() != expectedStrings[i]) {
            PluginTester::printError(QString("CRITICAL: Strings array validation failed at index %1").arg(i));
            logos_core_cleanup();
            exit(1);
        }
    }
    PluginTester::printSuccess("PASS: Strings array content validation");
    
    // Test objects array
    QVariant objectsResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getJsonArray", "objects");
    QJsonArray objectsArray = objectsResult.toJsonArray();
    QJsonObject firstObj = objectsArray[0].toObject();
    if (firstObj["name"].toString() != "John" || firstObj["age"].toInt() != 30) {
        PluginTester::printError("CRITICAL: Objects array validation failed for first object");
        logos_core_cleanup();
        exit(1);
    }
    PluginTester::printSuccess("PASS: Objects array content validation");

    // Test the new getStringList() method
    qDebug() << "\n=== Testing getStringList() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "getStringListTriggered", EventTracker::onEvent);
    
    // Test different list types
    QStringList listTypes = {"fruits", "colors", "countries", "programming", "numbers", "unknown"};
    QList<int> expectedListSizes = {5, 4, 4, 5, 5, 0}; // Expected list sizes for each type
    
    for (int i = 0; i < listTypes.size(); i++) {
        const QString& listType = listTypes[i];
        int expectedSize = expectedListSizes[i];
        
        qDebug() << "Testing getStringList() with listType:" << listType;
        QVariant listResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getStringList", listType);
        
        if (!listResult.isValid()) {
            PluginTester::printError(QString("CRITICAL: getStringList() failed for type: %1").arg(listType));
            logos_core_cleanup();
            exit(1);
        }
        
        // Convert QVariant to QStringList
        QStringList stringList = listResult.toStringList();
        if (stringList.size() != expectedSize) {
            PluginTester::printError(QString("CRITICAL: getStringList('%1') size mismatch. Expected: %2, Got: %3")
                                   .arg(listType).arg(expectedSize).arg(stringList.size()));
            logos_core_cleanup();
            exit(1);
        }
        
        PluginTester::printSuccess(QString("PASS: getStringList('%1') returned list of size %2")
                                 .arg(listType).arg(stringList.size()));
        
        // Print list contents for verification
        qDebug() << "List contents:" << stringList;
    }
    
    // Test specific list content validation
    qDebug() << "\n=== Validating specific list contents ===";
    
    // Test fruits list
    QVariant fruitsResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getStringList", "fruits");
    QStringList fruitsList = fruitsResult.toStringList();
    QStringList expectedFruits = {"apple", "banana", "cherry", "date", "elderberry"};
    for (int i = 0; i < expectedFruits.size(); i++) {
        if (fruitsList[i] != expectedFruits[i]) {
            PluginTester::printError(QString("CRITICAL: Fruits list validation failed at index %1").arg(i));
            logos_core_cleanup();
            exit(1);
        }
    }
    PluginTester::printSuccess("PASS: Fruits list content validation");
    
    // Test colors list
    QVariant colorsResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "getStringList", "colors");
    QStringList colorsList = colorsResult.toStringList();
    QStringList expectedColors = {"red", "green", "blue", "yellow"};
    for (int i = 0; i < expectedColors.size(); i++) {
        if (colorsList[i] != expectedColors[i]) {
            PluginTester::printError(QString("CRITICAL: Colors list validation failed at index %1").arg(i));
            logos_core_cleanup();
            exit(1);
        }
    }
    PluginTester::printSuccess("PASS: Colors list content validation");

    // Test the new processData() method with 3 parameters (string, int, string)
    qDebug() << "\n=== Testing processData() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "processDataTriggered", EventTracker::onEvent);
    
    QString title = "Temperature";
    int value = 23;
    QString unit = "°C";
    QString expectedProcessResult = QString("%1: %2 %3").arg(title).arg(value).arg(unit); // "Temperature: 23 °C"
    
    qDebug() << "Calling processData() with:" << title << value << unit;
    QVariant processResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "processData", title, value, unit);
    
    if (!processResult.isValid()) {
        PluginTester::printError("CRITICAL: Failed to call processData() method");
        logos_core_cleanup();
        exit(1);
    }
    
    QString actualProcessResult = processResult.toString();
    if (actualProcessResult != expectedProcessResult) {
        PluginTester::printError(QString("CRITICAL: processData() result mismatch. Expected: '%1', Got: '%2'")
                               .arg(expectedProcessResult).arg(actualProcessResult));
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess(QString("PASS: processData() returned correct result: '%1'").arg(actualProcessResult));
    
    // Process events and check for event
    QCoreApplication::processEvents();
    
    if (EventTracker::eventReceived) {
        PluginTester::printSuccess("SUCCESS: processData() event received!");
        if (EventTracker::lastEventData.size() >= 4 && 
            EventTracker::lastEventData[0].toString() == title &&
            EventTracker::lastEventData[1].toInt() == value &&
            EventTracker::lastEventData[2].toString() == unit &&
            EventTracker::lastEventData[3].toString() == expectedProcessResult) {
            PluginTester::printSuccess("SUCCESS: processData() event contains correct parameters");
        } else {
            PluginTester::printError("CRITICAL: processData() event parameter mismatch");
            qDebug() << "Expected:" << title << value << unit << expectedProcessResult;
            qDebug() << "Got:" << EventTracker::lastEventData;
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: No processData() event received");
        logos_core_cleanup();
        exit(1);
    }

    // Test the new combineStrings() method
    qDebug() << "\n=== Testing combineStrings() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "combineStringsTriggered", EventTracker::onEvent);
    
    QString str1 = "Hello";
    QString str2 = "World";
    QString expectedResult = str1 + " + " + str2; // "Hello + World"
    
    qDebug() << "Calling combineStrings() with:" << str1 << "and" << str2;
    QVariant combineResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "combineStrings", str1, str2);
    
    if (!combineResult.isValid()) {
        PluginTester::printError("CRITICAL: Failed to call combineStrings() method");
        logos_core_cleanup();
        exit(1);
    }
    
    QString actualResult = combineResult.toString();
    if (actualResult != expectedResult) {
        PluginTester::printError(QString("CRITICAL: combineStrings() result mismatch. Expected: '%1', Got: '%2'")
                               .arg(expectedResult).arg(actualResult));
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess(QString("PASS: combineStrings() returned correct result: '%1'").arg(actualResult));
    
    // Process events and check for event
    QCoreApplication::processEvents();
    
    if (EventTracker::eventReceived) {
        PluginTester::printSuccess("SUCCESS: combineStrings() event received!");
        if (EventTracker::lastEventData.size() >= 3 && 
            EventTracker::lastEventData[0].toString() == str1 &&
            EventTracker::lastEventData[1].toString() == str2 &&
            EventTracker::lastEventData[2].toString() == expectedResult) {
            PluginTester::printSuccess("SUCCESS: combineStrings() event contains correct parameters");
        } else {
            PluginTester::printError("CRITICAL: combineStrings() event parameter mismatch");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: No combineStrings() event received");
        logos_core_cleanup();
        exit(1);
    }

    // Test the new formatMessage() method with 3 arguments
    qDebug() << "\n=== Testing formatMessage() method ===";
    EventTracker::eventReceived = false;
    testAPI.getClient("template_module")->onEvent(templateModuleObj, nullptr, "formatMessageTriggered", EventTracker::onEvent);
    
    QString prefix = "INFO";
    QString message = "System started successfully";
    QString suffix = "OK";
    QString expectedResult_2 = QString("[%1] %2 [%3]").arg(prefix).arg(message).arg(suffix); // "[INFO] System started successfully [OK]"
    
    qDebug() << "Calling formatMessage() with:" << prefix << message << suffix;
    QVariant formatResult = testAPI.getClient("template_module")->invokeRemoteMethod("template_module", "formatMessage", prefix, message, suffix);
    
    if (!formatResult.isValid()) {
        PluginTester::printError("CRITICAL: Failed to call formatMessage() method");
        logos_core_cleanup();
        exit(1);
    }
    
    QString actualResult_2 = formatResult.toString();
    if (actualResult_2 != expectedResult_2) {
        PluginTester::printError(QString("CRITICAL: formatMessage() result mismatch. Expected: '%1', Got: '%2'")
                               .arg(expectedResult_2).arg(actualResult_2));
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess(QString("PASS: formatMessage() returned correct result: '%1'").arg(actualResult));
    
    // Process events and check for event
    QCoreApplication::processEvents();
    
    if (EventTracker::eventReceived) {
        PluginTester::printSuccess("SUCCESS: formatMessage() event received!");

        if (EventTracker::lastEventData.size() >= 4 && 
            EventTracker::lastEventData[0].toString() == prefix &&
            EventTracker::lastEventData[1].toString() == message &&
            EventTracker::lastEventData[2].toString() == suffix &&
            EventTracker::lastEventData[3].toString() == expectedResult_2) {
            PluginTester::printSuccess("SUCCESS: formatMessage() event contains correct parameters");
        } else {
            PluginTester::printError("CRITICAL: formatMessage() event parameter mismatch");
            logos_core_cleanup();
            exit(1);
        }
    } else {
        PluginTester::printError("CRITICAL: No formatMessage() event received");
        logos_core_cleanup();
        exit(1);
    }

    // Test the capability_module
    qDebug() << "\n=== Testing Capability Module ===";
    
    // Initialize LogosAPI for testing capability_module
    LogosAPI capabilityAPI("test_simple");
    
    // Get capability_module object for testing
    QObject* capabilityModuleObj = capabilityAPI.getClient("capability_module")->requestObject("capability_module");
    if (!capabilityModuleObj) {
        PluginTester::printError("CRITICAL: Failed to get capability_module from registry");
        logos_core_cleanup();
        exit(1);
    }
    
    // Test the requestModule method
    QString testModuleName = "test_module";
    qDebug() << "Calling requestModule() with parameter:" << testModuleName;
    
    QVariant requestResult = capabilityAPI.getClient("capability_module")->invokeRemoteMethod("capability_module", "requestModule", "app", testModuleName);
    if (!requestResult.isValid()) {
        PluginTester::printError("CRITICAL: Failed to call requestModule() method");
        logos_core_cleanup();
        exit(1);
    }
    
    QString actualCapabilityResult = requestResult.toString();
    QString expectedCapabilityResult = "abc"; // The hardcoded return value
    
    if (actualCapabilityResult != expectedCapabilityResult) {
        PluginTester::printError(QString("CRITICAL: requestModule() result mismatch. Expected: '%1', Got: '%2'")
                               .arg(expectedCapabilityResult).arg(actualCapabilityResult));
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess(QString("PASS: requestModule() returned correct result: '%1'").arg(actualCapabilityResult));

    // Test the new logos_core_get_token API
    qDebug() << "\n=== Testing logos_core_get_token API ===";
    
    // Test retrieving the token that was saved during core initialization
    const char* tokenKey = "core_manager";
    qDebug() << "Retrieving token for key:" << tokenKey;
    
    char* retrievedToken = logos_core_get_token(tokenKey);
    if (!retrievedToken) {
        PluginTester::printError(QString("CRITICAL: Failed to retrieve token for key: %1").arg(tokenKey));
        logos_core_cleanup();
        exit(1);
    }
    
    QString tokenValue = QString::fromUtf8(retrievedToken);
    QString expectedTokenValue = "abc"; // The token saved in logos_core.cpp line 390
    
    if (tokenValue != expectedTokenValue) {
        PluginTester::printError(QString("CRITICAL: Token value mismatch. Expected: '%1', Got: '%2'")
                               .arg(expectedTokenValue).arg(tokenValue));
        delete[] retrievedToken; // Clean up memory
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess(QString("PASS: logos_core_get_token() returned correct token: '%1'").arg(tokenValue));
    
    // Clean up the allocated memory
    delete[] retrievedToken;
    
    // Test retrieving a non-existent token
    const char* nonExistentKey = "non_existent_token";
    qDebug() << "Testing retrieval of non-existent token:" << nonExistentKey;
    
    char* nonExistentToken = logos_core_get_token(nonExistentKey);
    if (nonExistentToken != nullptr) {
        PluginTester::printError(QString("CRITICAL: Expected NULL for non-existent token, but got: '%1'")
                               .arg(QString::fromUtf8(nonExistentToken)));
        delete[] nonExistentToken;
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess("PASS: logos_core_get_token() correctly returned NULL for non-existent token");
    
    // Test passing NULL key
    qDebug() << "Testing logos_core_get_token() with NULL key";
    char* nullKeyToken = logos_core_get_token(nullptr);
    if (nullKeyToken != nullptr) {
        PluginTester::printError("CRITICAL: Expected NULL for NULL key, but got a token");
        delete[] nullKeyToken;
        logos_core_cleanup();
        exit(1);
    }
    
    PluginTester::printSuccess("PASS: logos_core_get_token() correctly returned NULL for NULL key");

    // All assertions passed - test completed successfully
    qDebug() << "\n=== All Tests Completed Successfully ===";
    qDebug() << "✓ Plugin loading tests passed";
    qDebug() << "✓ Event system tests passed";
    qDebug() << "✓ Template module foo() event trigger test passed";
    qDebug() << "✓ Template module bar() method test passed";
    qDebug() << "✓ Template module stringToBool() method test passed";
    qDebug() << "✓ Template module getJsonArray() method test passed";
    qDebug() << "✓ Array content validation tests passed";
    qDebug() << "✓ Template module combineStrings() method test passed";
    qDebug() << "✓ Template module getStringList() method test passed";
    qDebug() << "✓ Template module processData() method test passed";
    qDebug() << "✓ Template module formatMessage() method test passed";
    qDebug() << "✓ Capability module requestModule() method test passed";
    qDebug() << "✓ logos_core_get_token() API test passed";

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