#include <iostream>
#include <QDir>
#include <QString>
#include <QTimer>
#include <QCoreApplication>
#include "../../core/src/logos_core.h"

// Helper function to print loaded plugins
void printLoadedPlugins() {
    char** plugins = logos_core_get_loaded_plugins();
    
    if (plugins == nullptr || plugins[0] == nullptr) {
        std::cout << "No plugins loaded." << std::endl;
        delete[] plugins; // Free the empty array
        return;
    }
    
    // Count the plugins
    int count = 0;
    while (plugins[count] != nullptr) {
        count++;
    }
    
    std::cout << "Currently loaded plugins (" << count << "):" << std::endl;
    for (int i = 0; i < count; ++i) {
        std::cout << "  - " << plugins[i] << std::endl;
        
        // Free the memory for each string
        delete[] plugins[i];
    }
    
    // Free the array itself
    delete[] plugins;
}

int main(int argc, char *argv[])
{
    std::cout << "Custom Application Starting..." << std::endl;

    // Initialize the logos core library
    logos_core_init(argc, argv);

    std::cout << "Logos Core initialized successfully!" << std::endl;

    // Set the custom plugins directory
    QString pluginsDir = QDir::cleanPath(QDir::currentPath() + "/bin/plugins");
    std::cout << "Setting plugins directory to: " << pluginsDir.toStdString() << std::endl;
    logos_core_set_plugins_dir(pluginsDir.toUtf8().constData());

    // Start the logos core functionality
    logos_core_start();

    std::cout << "Logos Core started successfully!" << std::endl;

    // Print loaded plugins initially
    std::cout << "\nInitial plugin list:" << std::endl;
    printLoadedPlugins();

    // Create a timer to print message and plugin list every 5 seconds
    QTimer* timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, []() {
        std::cout << "\n--- Timer triggered ---" << std::endl;
        printLoadedPlugins();
    });
    timer->start(5000); // 5000ms (5 seconds)

    std::cout << "Timer started, application will continue running..." << std::endl;

    // IMPORTANT: We're using QCoreApplication::exec() directly instead of logos_core_exec()
    // because logos_core_exec() is just a wrapper around QCoreApplication::exec().
    // This approach allows us to set up our timer before entering the event loop,
    // ensuring that the timer will actually run.
    //
    // The logos_core_init() function creates a QCoreApplication instance internally,
    // so we can access the same event loop directly.
    int result = QCoreApplication::exec();

    // Clean up resources
    delete timer;
    logos_core_cleanup();

    return result;
} 
