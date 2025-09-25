#include "package_manager_plugin.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QPluginLoader>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaObject>
#include <QRemoteObjectNode>
#include <QRemoteObjectReplica>
#include <QRemoteObjectPendingCall>
#include "../../logos-cpp-sdk/cpp/logos_api_client.h"
#include "logos_sdk.h"

PackageManagerPlugin::PackageManagerPlugin()
{
    qDebug() << "PackageManagerPlugin created";
    qDebug() << "PackageManagerPlugin: LogosAPI initialized";
}

PackageManagerPlugin::~PackageManagerPlugin() 
{
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
}

bool PackageManagerPlugin::installPlugin(const QString& pluginPath)
{
    qDebug() << "PackageManager: Installing plugin:" << pluginPath;

    // Verify the source file exists
    QFileInfo sourceFileInfo(pluginPath);
    if (!sourceFileInfo.exists() || !sourceFileInfo.isFile()) {
        qWarning() << "Source plugin file does not exist or is not a file:" << pluginPath;
        return false;
    }

    // Use m_pluginsDirectory if set, otherwise default
    QString pluginsDirectory = m_pluginsDirectory.isEmpty()
        ? QDir::cleanPath(QCoreApplication::applicationDirPath() + "/bin/modules")
        : m_pluginsDirectory;
    qDebug() << "Plugins directory:" << pluginsDirectory;

    // Make sure we have a valid plugins directory
    if (pluginsDirectory.isEmpty()) {
        qWarning() << "Plugins directory is not set. Cannot install plugin.";
        return false;
    }

    // Create the plugins directory if it doesn't exist
    QDir pluginsDir(pluginsDirectory);
    if (!pluginsDir.exists()) {
        qDebug() << "Creating plugins directory:" << pluginsDirectory;
        if (!pluginsDir.mkpath(".")) {
            qWarning() << "Failed to create plugins directory:" << pluginsDirectory;
            return false;
        }
    }

    // Get the filename from the source path
    QString fileName = sourceFileInfo.fileName();
    QString destinationPath = pluginsDir.filePath(fileName);

    // Check if the destination file already exists
    QFileInfo destFileInfo(destinationPath);
    if (destFileInfo.exists()) {
        qDebug() << "Plugin already exists at destination. Overwriting:" << destinationPath;
        QFile destFile(destinationPath);
        if (!destFile.remove()) {
            qWarning() << "Failed to remove existing plugin file:" << destinationPath;
            return false;
        }
    }

    // Copy the plugin file to the plugins directory
    QFile sourceFile(pluginPath);
    if (!sourceFile.copy(destinationPath)) {
        qWarning() << "Failed to copy plugin file to plugins directory:" 
                  << sourceFile.errorString();
        return false;
    }

    qDebug() << "Successfully installed plugin:" << fileName << "to" << destinationPath;
    
    // Read the plugin metadata to check for included files
    QPluginLoader loader(pluginPath);
    QJsonObject metadata = loader.metaData();
    if (!metadata.isEmpty()) {
        QJsonObject metaDataObj = metadata.value("MetaData").toObject();
        QJsonArray includeFiles = metaDataObj.value("include").toArray();
        
        if (!includeFiles.isEmpty()) {
            qDebug() << "Plugin has" << includeFiles.size() << "included files to copy";
            
            // Get the source directory (where the plugin is)
            QDir sourceDir = sourceFileInfo.dir();
            
            // Try to copy each included file
            for (const QJsonValue& includeVal : includeFiles) {
                QString includeFileName = includeVal.toString();
                if (includeFileName.isEmpty()) continue;
                
                QString sourceIncludePath = sourceDir.filePath(includeFileName);
                QString destIncludePath = pluginsDir.filePath(includeFileName);
                
                qDebug() << "Checking for included file:" << sourceIncludePath;
                
                // Check if the source file exists
                QFileInfo includeFileInfo(sourceIncludePath);
                if (includeFileInfo.exists() && includeFileInfo.isFile()) {
                    qDebug() << "Found included file:" << sourceIncludePath;
                    
                    // Check if the destination file already exists
                    QFileInfo destIncludeFileInfo(destIncludePath);
                    if (destIncludeFileInfo.exists()) {
                        qDebug() << "Included file already exists at destination. Overwriting:" << destIncludePath;
                        QFile destIncludeFile(destIncludePath);
                        if (!destIncludeFile.remove()) {
                            qWarning() << "Failed to remove existing included file:" << destIncludePath;
                            // Continue anyway - this isn't a fatal error
                        }
                    }
                    
                    // Copy the included file
                    QFile includeFile(sourceIncludePath);
                    if (includeFile.copy(destIncludePath)) {
                        qDebug() << "Successfully copied included file:" << includeFileName;
                    } else {
                        qWarning() << "Failed to copy included file:" << includeFileName 
                                  << "-" << includeFile.errorString();
                        // Continue anyway - this isn't a fatal error
                    }
                } else {
                    qDebug() << "Included file not found:" << sourceIncludePath;
                    // It's ok if the file doesn't exist - continue with other files
                }
            }
        }
    }
    
    // Use LogosAPI to call the remote method
    if (!logosAPI) {
        qWarning() << "Failed to connect to Logos Core registry.";
        return false;
    }

    LogosAPIClient* coreManagerClient = logosAPI->getClient("core_manager");
    if (!coreManagerClient || !coreManagerClient->isConnected()) {
        qWarning() << "Failed to connect to Logos Core registry.";
        return false;
    }

    LogosModules logos(logosAPI);

    qDebug() << "Calling processPlugin with destinationPath:" << destinationPath;
    QString pluginName = logos.core_manager.processPlugin(destinationPath);
    if (pluginName.isEmpty()) {
        qDebug() << "ERROR: --------------------------------";
        qWarning() << "Failed to process installed plugin:" << destinationPath;
        return false;
    }
    qDebug() << "Successfully processed installed plugin:" << pluginName;
    return true;
}

QJsonArray PackageManagerPlugin::getPackages() {
    QJsonArray packagesArray;

    // Get the application directory path
    QString appDir = QCoreApplication::applicationDirPath();
    QDir packagesDir(appDir + "/packages");

    // Check if the packages directory exists
    if (!packagesDir.exists()) {
        qDebug() << "Packages directory not found at:" << packagesDir.absolutePath();
        return packagesArray;
    }

    // Get all plugin files (*.so, *.dll, *.dylib) from the packages directory
    QStringList nameFilters;
#ifdef Q_OS_WIN
    nameFilters << "*.dll";
#elif defined(Q_OS_MAC)
    nameFilters << "*.dylib";
#else
    nameFilters << "*.so";
#endif
    QStringList pluginFiles = packagesDir.entryList(nameFilters, QDir::Files);

    for (const QString& fileName : pluginFiles) {
        QString filePath = packagesDir.absoluteFilePath(fileName);
        QPluginLoader loader(filePath);
        QJsonObject metadata = loader.metaData();
        if (metadata.isEmpty()) {
            qDebug() << "Failed to load metadata from:" << filePath;
            continue;
        }
        QJsonObject root = metadata.value("MetaData").toObject();
        QString name = root.value("name").toString(fileName);
        QString version = root.value("version").toString("1.0.0");
        QString description = root.value("description").toString("Qt Plugin");
        QString category = root.value("category").toString("Uncategorized");
        QString type = root.value("type").toString("Plugin");
        QJsonArray depsArray = root.value("dependencies").toArray();
        QJsonArray dependencies;
        for (const QJsonValue& dep : depsArray) {
            dependencies.append(dep.toString());
        }
        QJsonObject packageObj;
        packageObj["name"] = name;
        packageObj["installedVersion"] = version;
        packageObj["latestVersion"] = version;
        packageObj["description"] = description;
        packageObj["category"] = category;
        packageObj["type"] = type;
        packageObj["path"] = filePath;
        packageObj["dependencies"] = dependencies;
        packagesArray.append(packageObj);
    }
    
    return packagesArray;
}

void PackageManagerPlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
}
