#include <QCoreApplication>
#include <QPluginLoader>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaMethod>
#include <QByteArray>
#include <QTextStream>
#include <QDir>
#include <QByteArrayList>
#include <QFile>
#include <QSet>
#include <QtGlobal>

static QJsonArray enumerateMethods(QObject* moduleInstance)
{
    QJsonArray methodsArray;

    if (!moduleInstance) {
        return methodsArray;
    }

    const QMetaObject* metaObject = moduleInstance->metaObject();

    for (int i = 0; i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);

        if (method.enclosingMetaObject() != metaObject) {
            continue;
        }

        QJsonObject methodObj;
        methodObj["signature"] = QString::fromUtf8(method.methodSignature());
        methodObj["name"] = QString::fromUtf8(method.name());
        methodObj["returnType"] = QString::fromUtf8(method.typeName());
        bool isInvokable = method.isValid() && (method.methodType() == QMetaMethod::Method || method.methodType() == QMetaMethod::Slot);
        methodObj["isInvokable"] = isInvokable;

        if (method.parameterCount() > 0) {
            QJsonArray params;
            for (int p = 0; p < method.parameterCount(); ++p) {
                QJsonObject paramObj;
                paramObj["type"] = QString::fromUtf8(method.parameterTypeName(p));
                QByteArrayList paramNames = method.parameterNames();
                if (p < paramNames.size() && !paramNames.at(p).isEmpty()) {
                    paramObj["name"] = QString::fromUtf8(paramNames.at(p));
                } else {
                    paramObj["name"] = QString("param%1").arg(p);
                }
                params.append(paramObj);
            }
            methodObj["parameters"] = params;
        }

        methodsArray.append(methodObj);
    }

    return methodsArray;
}

static QString toPascalCase(const QString& name)
{
    QString out;
    bool cap = true;
    for (QChar c : name) {
        if (!c.isLetterOrNumber()) { cap = true; continue; }
        if (cap) { out.append(c.toUpper()); cap = false; }
        else { out.append(c.toLower()); }
    }
    if (out.isEmpty()) return QString("Module");
    return out;
}

static QString normalizeType(QString t)
{
    t = t.trimmed();
    if (t.startsWith("const ")) t = t.mid(6);
    t = t.trimmed();
    // Drop reference and pointer qualifiers
    if (t.endsWith('&') || t.endsWith('*')) t.chop(1);
    t = t.trimmed();
    return t;
}

static QString mapParamType(const QString& qtType)
{
    const QString base = normalizeType(qtType);
    static const QSet<QString> known = {
        "void","bool","int","double","float","QString","QStringList","QJsonArray","QVariant"
    };
    if (known.contains(base)) return base;
    // Fallback to QVariant for unknown types
    return QString("QVariant");
}

static QString mapReturnType(const QString& qtType)
{
    const QString base = normalizeType(qtType);
    if (base.isEmpty() || base == "void") return QString("void");
    static const QSet<QString> known = {
        "bool","int","double","float","QString","QStringList","QJsonArray","QVariant"
    };
    if (known.contains(base)) return base;
    return QString("QVariant");
}

static QString makeHeader(const QString& moduleName, const QString& className, const QJsonArray& methods)
{
    QString h;
    QTextStream s(&h);
    s << "#pragma once\n";
    s << "#include <QString>\n";
    s << "#include <QVariant>\n";
    s << "#include <QStringList>\n";
    s << "#include <QJsonArray>\n";
    s << "#include <QObject>\n";
    s << "#include <QPointer>\n";
    s << "#include <functional>\n";
    s << "#include <utility>\n";
    s << "#include \"logos_api.h\"\n";
    s << "#include \"logos_api_client.h\"\n\n";
    s << "class " << className << " {\n";
    s << "public:\n";
    s << "    explicit " << className << "(LogosAPI* api);\n\n";
    s << "    using RawEventCallback = std::function<void(const QString&, const QVariantList&)>;\n";
    s << "    using EventCallback = std::function<void(const QVariantList&)>;\n\n";
    s << "    bool on(const QString& eventName, RawEventCallback callback);\n";
    s << "    bool on(const QString& eventName, EventCallback callback);\n";
    s << "    void setEventSource(QObject* source);\n";
    s << "    QObject* eventSource() const;\n";
    s << "    void trigger(const QString& eventName);\n";
    s << "    void trigger(const QString& eventName, const QVariantList& data);\n";
    s << "    template<typename... Args>\n";
    s << "    void trigger(const QString& eventName, Args&&... args) {\n";
    s << "        trigger(eventName, packVariantList(std::forward<Args>(args)...));\n";
    s << "    }\n";
    s << "    void trigger(const QString& eventName, QObject* source, const QVariantList& data);\n";
    s << "    template<typename... Args>\n";
    s << "    void trigger(const QString& eventName, QObject* source, Args&&... args) {\n";
    s << "        trigger(eventName, source, packVariantList(std::forward<Args>(args)...));\n";
    s << "    }\n\n";
    // Methods
    for (const QJsonValue& v : methods) {
        const QJsonObject o = v.toObject();
        const bool invokable = o.value("isInvokable").toBool();
        if (!invokable) continue;
        const QString name = o.value("name").toString();
        const QString ret = mapReturnType(o.value("returnType").toString());
        s << "    " << ret << " " << name << "(";
        QJsonArray params = o.value("parameters").toArray();
        for (int i = 0; i < params.size(); ++i) {
            QJsonObject p = params.at(i).toObject();
            QString pt = mapParamType(p.value("type").toString());
            QString pn = p.value("name").toString();
            if (pt == "QString" || pt == "QStringList" || pt == "QJsonArray") {
                s << "const " << pt << "& " << pn;
            } else {
                s << pt << " " << pn;
            }
            if (i + 1 < params.size()) s << ", ";
        }
        s << ");\n";
    }
    s << "\nprivate:\n";
    s << "    QObject* ensureReplica();\n";
    s << "    template<typename... Args>\n";
    s << "    static QVariantList packVariantList(Args&&... args) {\n";
    s << "        QVariantList list;\n";
    s << "        list.reserve(sizeof...(Args));\n";
    s << "        using Expander = int[];\n";
    s << "        (void)Expander{0, (list.append(QVariant::fromValue(std::forward<Args>(args))), 0)...};\n";
    s << "        return list;\n";
    s << "    }\n";
    s << "    LogosAPI* m_api;\n";
    s << "    LogosAPIClient* m_client;\n";
    s << "    QString m_moduleName;\n";
    s << "    QPointer<QObject> m_eventReplica;\n";
    s << "    QPointer<QObject> m_eventSource;\n";
    s << "};\n";
    return h;
}

static QString makeSource(const QString& moduleName, const QString& className, const QString& headerBaseName, const QJsonArray& methods)
{
    QString c;
    QTextStream s(&c);
    s << "#include \"" << headerBaseName << "\"\n\n";
    s << "#include <QDebug>\n\n";
    s << className << "::" << className << "(LogosAPI* api) : m_api(api), m_client(api->getClient(\"" << moduleName << "\")), m_moduleName(QStringLiteral(\"" << moduleName << "\")) {}\n\n";
    s << "QObject* " << className << "::ensureReplica() {\n";
    s << "    if (!m_eventReplica) {\n";
    s << "        QObject* replica = m_client->requestObject(m_moduleName);\n";
    s << "        if (!replica) {\n";
    s << "            qWarning() << \"" << className << ": failed to acquire remote object for events on\" << m_moduleName;\n";
    s << "            return nullptr;\n";
    s << "        }\n";
    s << "        m_eventReplica = replica;\n";
    s << "    }\n";
    s << "    return m_eventReplica.data();\n";
    s << "}\n\n";
    s << "bool " << className << "::on(const QString& eventName, RawEventCallback callback) {\n";
    s << "    if (!callback) {\n";
    s << "        qWarning() << \"" << className << ": ignoring empty event callback for\" << eventName;\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    QObject* origin = ensureReplica();\n";
    s << "    if (!origin) {\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    m_client->onEvent(origin, nullptr, eventName, callback);\n";
    s << "    return true;\n";
    s << "}\n\n";
    s << "bool " << className << "::on(const QString& eventName, EventCallback callback) {\n";
    s << "    if (!callback) {\n";
    s << "        qWarning() << \"" << className << ": ignoring empty event callback for\" << eventName;\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    return on(eventName, [callback](const QString&, const QVariantList& data) {\n";
    s << "        callback(data);\n";
    s << "    });\n";
    s << "}\n\n";
    s << "void " << className << "::setEventSource(QObject* source) {\n";
    s << "    m_eventSource = source;\n";
    s << "}\n\n";
    s << "QObject* " << className << "::eventSource() const {\n";
    s << "    return m_eventSource.data();\n";
    s << "}\n\n";
    s << "void " << className << "::trigger(const QString& eventName) {\n";
    s << "    trigger(eventName, QVariantList{});\n";
    s << "}\n\n";
    s << "void " << className << "::trigger(const QString& eventName, const QVariantList& data) {\n";
    s << "    if (!m_eventSource) {\n";
    s << "        qWarning() << \"" << className << ": no event source set for trigger\" << eventName;\n";
    s << "        return;\n";
    s << "    }\n";
    s << "    m_client->onEventResponse(m_eventSource.data(), eventName, data);\n";
    s << "}\n\n";
    s << "void " << className << "::trigger(const QString& eventName, QObject* source, const QVariantList& data) {\n";
    s << "    if (!source) {\n";
    s << "        qWarning() << \"" << className << ": cannot trigger\" << eventName << \"with null source\";\n";
    s << "        return;\n";
    s << "    }\n";
    s << "    m_client->onEventResponse(source, eventName, data);\n";
    s << "}\n\n";
    for (const QJsonValue& v : methods) {
        const QJsonObject o = v.toObject();
        const bool invokable = o.value("isInvokable").toBool();
        if (!invokable) continue;
        const QString name = o.value("name").toString();
        const QString ret = mapReturnType(o.value("returnType").toString());
        QJsonArray params = o.value("parameters").toArray();
        // Signature
        s << ret << " " << className << "::" << name << "(";
        for (int i = 0; i < params.size(); ++i) {
            QJsonObject p = params.at(i).toObject();
            QString pt = mapParamType(p.value("type").toString());
            QString pn = p.value("name").toString();
            if (pt == "QString" || pt == "QStringList" || pt == "QJsonArray") {
                s << "const " << pt << "& " << pn;
            } else {
                s << pt << " " << pn;
            }
            if (i + 1 < params.size()) s << ", ";
        }
        s << ") {\n";
        // Body: perform call
        if (ret != "void") {
            s << "    QVariant _result = ";
        } else {
            s << "    ";
        }
        if (params.size() == 0) {
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\");\n";
        } else if (params.size() == 1) {
            QJsonObject p = params.at(0).toObject();
            QString pn = p.value("name").toString();
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", " << pn << ");\n";
        } else if (params.size() == 2) {
            QString p0 = params.at(0).toObject().value("name").toString();
            QString p1 = params.at(1).toObject().value("name").toString();
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", " << p0 << ", " << p1 << ");\n";
        } else if (params.size() == 3) {
            QString p0 = params.at(0).toObject().value("name").toString();
            QString p1 = params.at(1).toObject().value("name").toString();
            QString p2 = params.at(2).toObject().value("name").toString();
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", " << p0 << ", " << p1 << ", " << p2 << ");\n";
        } else if (params.size() == 4) {
            QString p0 = params.at(0).toObject().value("name").toString();
            QString p1 = params.at(1).toObject().value("name").toString();
            QString p2 = params.at(2).toObject().value("name").toString();
            QString p3 = params.at(3).toObject().value("name").toString();
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", " << p0 << ", " << p1 << ", " << p2 << ", " << p3 << ");\n";
        } else if (params.size() == 5) {
            QString p0 = params.at(0).toObject().value("name").toString();
            QString p1 = params.at(1).toObject().value("name").toString();
            QString p2 = params.at(2).toObject().value("name").toString();
            QString p3 = params.at(3).toObject().value("name").toString();
            QString p4 = params.at(4).toObject().value("name").toString();
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", " << p0 << ", " << p1 << ", " << p2 << ", " << p3 << ", " << p4 << ");\n";
        } else {
            s << "m_client->invokeRemoteMethod(\"" << moduleName << "\", \"" << name << "\", QVariantList{";
            for (int i = 0; i < params.size(); ++i) {
                QString pn = params.at(i).toObject().value("name").toString();
                s << pn;
                if (i + 1 < params.size()) s << ", ";
            }
            s << "});\n";
        }
        // Return conversion
        if (ret == "void") {
            // nothing
        } else if (ret == "bool") {
            s << "    return _result.toBool();\n";
        } else if (ret == "int") {
            s << "    return _result.toInt();\n";
        } else if (ret == "double") {
            s << "    return _result.toDouble();\n";
        } else if (ret == "float") {
            s << "    return _result.toFloat();\n";
        } else if (ret == "QString") {
            s << "    return _result.toString();\n";
        } else if (ret == "QStringList") {
            s << "    return _result.toStringList();\n";
        } else if (ret == "QJsonArray") {
            s << "    return qvariant_cast<QJsonArray>(_result);\n";
        } else { // QVariant
            s << "    return _result;\n";
        }
        s << "}\n\n";
    }
    return c;
}

static QString makeCoreManagerHeader()
{
    QString h;
    QTextStream s(&h);
    s << "#pragma once\n";
    s << "#include <QString>\n";
    s << "#include <QVariant>\n";
    s << "#include <QStringList>\n";
    s << "#include <QJsonArray>\n";
    s << "#include <QObject>\n";
    s << "#include <QPointer>\n";
    s << "#include <functional>\n";
    s << "#include <utility>\n";
    s << "#include \"logos_api.h\"\n";
    s << "#include \"logos_api_client.h\"\n\n";
    s << "class CoreManager {\n";
    s << "public:\n";
    s << "    explicit CoreManager(LogosAPI* api);\n\n";
    s << "    using RawEventCallback = std::function<void(const QString&, const QVariantList&)>;\n";
    s << "    using EventCallback = std::function<void(const QVariantList&)>;\n\n";
    s << "    bool on(const QString& eventName, RawEventCallback callback);\n";
    s << "    bool on(const QString& eventName, EventCallback callback);\n";
    s << "    void setEventSource(QObject* source);\n";
    s << "    QObject* eventSource() const;\n";
    s << "    void trigger(const QString& eventName);\n";
    s << "    void trigger(const QString& eventName, const QVariantList& data);\n";
    s << "    template<typename... Args>\n";
    s << "    void trigger(const QString& eventName, Args&&... args) {\n";
    s << "        trigger(eventName, packVariantList(std::forward<Args>(args)...));\n";
    s << "    }\n";
    s << "    void trigger(const QString& eventName, QObject* source, const QVariantList& data);\n";
    s << "    template<typename... Args>\n";
    s << "    void trigger(const QString& eventName, QObject* source, Args&&... args) {\n";
    s << "        trigger(eventName, source, packVariantList(std::forward<Args>(args)...));\n";
    s << "    }\n\n";
    s << "    void initialize(int argc, char* argv[]);\n";
    s << "    void setPluginsDirectory(const QString& directory);\n";
    s << "    void start();\n";
    s << "    void cleanup();\n";
    s << "    QStringList getLoadedPlugins();\n";
    s << "    QJsonArray getKnownPlugins();\n";
    s << "    QJsonArray getPluginMethods(const QString& pluginName);\n";
    s << "    void helloWorld();\n";
    s << "    bool loadPlugin(const QString& pluginName);\n";
    s << "    bool unloadPlugin(const QString& pluginName);\n";
    s << "    QString processPlugin(const QString& filePath);\n\n";
    s << "private:\n";
    s << "    QObject* ensureReplica();\n";
    s << "    template<typename... Args>\n";
    s << "    static QVariantList packVariantList(Args&&... args) {\n";
    s << "        QVariantList list;\n";
    s << "        list.reserve(sizeof...(Args));\n";
    s << "        using Expander = int[];\n";
    s << "        (void)Expander{0, (list.append(QVariant::fromValue(std::forward<Args>(args))), 0)...};\n";
    s << "        return list;\n";
    s << "    }\n";
    s << "    LogosAPI* m_api;\n";
    s << "    LogosAPIClient* m_client;\n";
    s << "    QString m_moduleName;\n";
    s << "    QPointer<QObject> m_eventReplica;\n";
    s << "    QPointer<QObject> m_eventSource;\n";
    s << "};\n";
    return h;
}

static QString makeCoreManagerSource(const QString& headerBaseName)
{
    QString c;
    QTextStream s(&c);
    s << "#include \"" << headerBaseName << "\"\n\n";
    s << "#include <QDebug>\n";
    s << "#include <QStringList>\n\n";
    s << "CoreManager::CoreManager(LogosAPI* api) : m_api(api), m_client(api->getClient(\"core_manager\")), m_moduleName(QStringLiteral(\"core_manager\")) {}\n\n";
    s << "QObject* CoreManager::ensureReplica() {\n";
    s << "    if (!m_eventReplica) {\n";
    s << "        QObject* replica = m_client->requestObject(m_moduleName);\n";
    s << "        if (!replica) {\n";
    s << "            qWarning() << \"CoreManager: failed to acquire remote object for events on\" << m_moduleName;\n";
    s << "            return nullptr;\n";
    s << "        }\n";
    s << "        m_eventReplica = replica;\n";
    s << "    }\n";
    s << "    return m_eventReplica.data();\n";
    s << "}\n\n";
    s << "bool CoreManager::on(const QString& eventName, RawEventCallback callback) {\n";
    s << "    if (!callback) {\n";
    s << "        qWarning() << \"CoreManager: ignoring empty event callback for\" << eventName;\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    QObject* origin = ensureReplica();\n";
    s << "    if (!origin) {\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    m_client->onEvent(origin, nullptr, eventName, callback);\n";
    s << "    return true;\n";
    s << "}\n\n";
    s << "bool CoreManager::on(const QString& eventName, EventCallback callback) {\n";
    s << "    if (!callback) {\n";
    s << "        qWarning() << \"CoreManager: ignoring empty event callback for\" << eventName;\n";
    s << "        return false;\n";
    s << "    }\n";
    s << "    return on(eventName, [callback](const QString&, const QVariantList& data) {\n";
    s << "        callback(data);\n";
    s << "    });\n";
    s << "}\n\n";
    s << "void CoreManager::setEventSource(QObject* source) {\n";
    s << "    m_eventSource = source;\n";
    s << "}\n\n";
    s << "QObject* CoreManager::eventSource() const {\n";
    s << "    return m_eventSource.data();\n";
    s << "}\n\n";
    s << "void CoreManager::trigger(const QString& eventName) {\n";
    s << "    trigger(eventName, QVariantList{});\n";
    s << "}\n\n";
    s << "void CoreManager::trigger(const QString& eventName, const QVariantList& data) {\n";
    s << "    if (!m_eventSource) {\n";
    s << "        qWarning() << \"CoreManager: no event source set for trigger\" << eventName;\n";
    s << "        return;\n";
    s << "    }\n";
    s << "    m_client->onEventResponse(m_eventSource.data(), eventName, data);\n";
    s << "}\n\n";
    s << "void CoreManager::trigger(const QString& eventName, QObject* source, const QVariantList& data) {\n";
    s << "    if (!source) {\n";
    s << "        qWarning() << \"CoreManager: cannot trigger\" << eventName << \"with null source\";\n";
    s << "        return;\n";
    s << "    }\n";
    s << "    m_client->onEventResponse(source, eventName, data);\n";
    s << "}\n\n";
    s << "void CoreManager::initialize(int argc, char* argv[]) {\n";
    s << "    QStringList args;\n";
    s << "    if (argv) {\n";
    s << "        for (int i = 0; i < argc; ++i) {\n";
    s << "            args << QString::fromUtf8(argv[i] ? argv[i] : \"\");\n";
    s << "        }\n";
    s << "    }\n";
    s << "    m_client->invokeRemoteMethod(\"core_manager\", \"initialize\", argc, args);\n";
    s << "}\n\n";
    s << "void CoreManager::setPluginsDirectory(const QString& directory) {\n";
    s << "    m_client->invokeRemoteMethod(\"core_manager\", \"setPluginsDirectory\", directory);\n";
    s << "}\n\n";
    s << "void CoreManager::start() {\n";
    s << "    m_client->invokeRemoteMethod(\"core_manager\", \"start\");\n";
    s << "}\n\n";
    s << "void CoreManager::cleanup() {\n";
    s << "    m_client->invokeRemoteMethod(\"core_manager\", \"cleanup\");\n";
    s << "}\n\n";
    s << "QStringList CoreManager::getLoadedPlugins() {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"getLoadedPlugins\");\n";
    s << "    return _result.toStringList();\n";
    s << "}\n\n";
    s << "QJsonArray CoreManager::getKnownPlugins() {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"getKnownPlugins\");\n";
    s << "    return qvariant_cast<QJsonArray>(_result);\n";
    s << "}\n\n";
    s << "QJsonArray CoreManager::getPluginMethods(const QString& pluginName) {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"getPluginMethods\", pluginName);\n";
    s << "    return qvariant_cast<QJsonArray>(_result);\n";
    s << "}\n\n";
    s << "void CoreManager::helloWorld() {\n";
    s << "    m_client->invokeRemoteMethod(\"core_manager\", \"helloWorld\");\n";
    s << "}\n\n";
    s << "bool CoreManager::loadPlugin(const QString& pluginName) {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"loadPlugin\", pluginName);\n";
    s << "    return _result.toBool();\n";
    s << "}\n\n";
    s << "bool CoreManager::unloadPlugin(const QString& pluginName) {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"unloadPlugin\", pluginName);\n";
    s << "    return _result.toBool();\n";
    s << "}\n\n";
    s << "QString CoreManager::processPlugin(const QString& filePath) {\n";
    s << "    QVariant _result = m_client->invokeRemoteMethod(\"core_manager\", \"processPlugin\", filePath);\n";
    s << "    return _result.toString();\n";
    s << "}\n\n";
    return c;
}

static bool ensureCoreManagerWrapper(const QString& genDirPath, QTextStream& err)
{
    const QString headerRel = QStringLiteral("core_manager_api.h");
    const QString sourceRel = QStringLiteral("core_manager_api.cpp");
    const QString headerAbs = QDir(genDirPath).filePath(headerRel);
    const QString sourceAbs = QDir(genDirPath).filePath(sourceRel);

    QString header = makeCoreManagerHeader();
    QString source = makeCoreManagerSource(headerRel);

    QFile headerFile(headerAbs);
    if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        err << "Failed to write core manager header: " << headerAbs << "\n";
        return false;
    }
    headerFile.write(header.toUtf8());
    headerFile.close();

    QFile sourceFile(sourceAbs);
    if (!sourceFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        err << "Failed to write core manager source: " << sourceAbs << "\n";
        return false;
    }
    sourceFile.write(source.toUtf8());
    sourceFile.close();

    return true;
}

static bool writeUmbrellaHeader(const QString& genDirPath, QTextStream& err)
{
    // Generate logos-cpp-sdk/cpp/generated/logos_sdk.h that includes all *_api.h in this dir
    QDir genDir(genDirPath);
    QStringList headers = genDir.entryList(QStringList() << "*_api.h", QDir::Files | QDir::Readable);
    QString content;
    QTextStream s(&content);
    s << "#pragma once\n";
    s << "#include \"logos_api.h\"\n";
    s << "#include \"logos_api_client.h\"\n\n";
    // Includes
    for (const QString& h : headers) {
        s << "#include \"" << h << "\"\n";
    }
    s << "\n";
    // Convenience aggregator exposing module wrappers
    s << "struct LogosModules {\n";
    s << "    explicit LogosModules(LogosAPI* api) : api(api)";
    for (const QString& h : headers) {
        QString base = h;
        base.chop(QString("_api.h").size());
        QString className = toPascalCase(base);
        s << ", \n        " << base << "(api)";
    }
    s << " {}\n";
    s << "    LogosAPI* api;\n";
    for (const QString& h : headers) {
        QString base = h;
        base.chop(QString("_api.h").size());
        QString className = toPascalCase(base);
        s << "    " << className << " " << base << ";\n";
    }
    s << "};\n";

    QFile outFile(genDir.filePath("logos_sdk.h"));
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        err << "Failed to write umbrella header: " << outFile.fileName() << "\n";
        return false;
    }
    outFile.write(content.toUtf8());
    outFile.close();
    return true;
}

static bool writeUmbrellaSource(const QString& genDirPath, QTextStream& err)
{
    // Generate logos-cpp-sdk/cpp/generated/logos_sdk.cpp that includes all *_api.cpp in this dir
    QDir genDir(genDirPath);
    QStringList sources = genDir.entryList(QStringList() << "*_api.cpp", QDir::Files | QDir::Readable);
    QString content;
    QTextStream s(&content);
    s << "#include \"logos_sdk.h\"\n\n";
    for (const QString& c : sources) {
        s << "#include \"" << c << "\"\n";
    }
    s << "\n";

    QFile outFile(genDir.filePath("logos_sdk.cpp"));
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        err << "Failed to write umbrella source: " << outFile.fileName() << "\n";
        return false;
    }
    outFile.write(content.toUtf8());
    outFile.close();
    return true;
}

static int generateFromPlugin(const QString& pluginInputPath, QTextStream& out, QTextStream& err)
{
    QFileInfo fi(pluginInputPath);
    if (!fi.exists()) {
        err << "Plugin file does not exist: " << pluginInputPath << "\n";
        return 2;
    }

    QString resolvedPath = fi.canonicalFilePath();
    if (resolvedPath.isEmpty()) {
        resolvedPath = fi.absoluteFilePath();
    }

    QString genDirPath = QDir::current().filePath("logos-cpp-sdk/cpp/generated");
    QDir().mkpath(genDirPath);
    if (!ensureCoreManagerWrapper(genDirPath, err)) {
        return 9;
    }

    QPluginLoader loader(resolvedPath);
    if (!loader.load()) {
        err << "Failed to load plugin at " << resolvedPath << ": " << loader.errorString() << "\n";
        return 3;
    }
    QObject* instance = loader.instance();
    if (!instance) {
        err << "Plugin loaded but no instance could be created for " << resolvedPath << "\n";
        loader.unload();
        return 4;
    }

    QString moduleName;
    {
        QJsonObject md = loader.metaData();
        QJsonObject meta = md.value("MetaData").toObject();
        moduleName = meta.value("name").toString();
        if (moduleName.isEmpty()) {
            moduleName = QFileInfo(resolvedPath).baseName();
        }
    }

    QJsonArray methods = enumerateMethods(instance);

    QString className = toPascalCase(moduleName);
    QString headerRel = QString("%1_api.h").arg(moduleName);
    QString sourceRel = QString("%1_api.cpp").arg(moduleName);
    QString headerAbs = QDir(genDirPath).filePath(headerRel);
    QString sourceAbs = QDir(genDirPath).filePath(sourceRel);

    QString header = makeHeader(moduleName, className, methods);
    QString source = makeSource(moduleName, className, headerRel, methods);

    {
        QFile f(headerAbs);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            err << "Failed to write header: " << headerAbs << "\n";
            loader.unload();
            return 5;
        }
        f.write(header.toUtf8());
        f.close();
    }
    {
        QFile f(sourceAbs);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            err << "Failed to write source: " << sourceAbs << "\n";
            loader.unload();
            return 6;
        }
        f.write(source.toUtf8());
        f.close();
    }

    if (!writeUmbrellaHeader(genDirPath, err)) {
        loader.unload();
        return 7;
    }
    if (!writeUmbrellaSource(genDirPath, err)) {
        loader.unload();
        return 8;
    }

    QJsonDocument doc(methods);
    // out << doc.toJson(QJsonDocument::Indented) << "\n";
    out << "Generated: logos-cpp-sdk/cpp/generated/" << headerRel << " and logos-cpp-sdk/cpp/generated/" << sourceRel << "\n";
    out.flush();

    loader.unload();
    return 0;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTextStream err(stderr);
    QTextStream out(stdout);

    // Support: extract dependencies from a metadata.json file
    {
        const QStringList args = app.arguments();
        const int metaIdx = args.indexOf("--metadata");
        if (metaIdx != -1) {
            if (metaIdx + 1 >= args.size()) {
                err << "Usage: " << QFileInfo(app.applicationFilePath()).fileName() << " --metadata /absolute/path/to/metadata.json\n";
                return 1;
            }
            QString metaPathArg = args.at(metaIdx + 1);
            if (metaPathArg.startsWith('@')) {
                metaPathArg.remove(0, 1);
            }
            QFileInfo mfi(metaPathArg);
            if (!mfi.exists()) {
                err << "Metadata file does not exist: " << metaPathArg << "\n";
                return 2;
            }
            QString metaResolvedPath = mfi.canonicalFilePath();
            if (metaResolvedPath.isEmpty()) {
                metaResolvedPath = mfi.absoluteFilePath();
            }
            QFile mf(metaResolvedPath);
            if (!mf.open(QIODevice::ReadOnly | QIODevice::Text)) {
                err << "Failed to open metadata file: " << metaResolvedPath << "\n";
                return 3;
            }
            const QByteArray jsonData = mf.readAll();
            mf.close();
            QJsonParseError parseError;
            const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
                err << "Invalid metadata JSON in " << metaResolvedPath << ": " << parseError.errorString() << "\n";
                return 4;
            }
            const QJsonObject obj = doc.object();
            const QJsonArray deps = obj.value("dependencies").toArray();

            // If --module-dir provided, generate for each dependency; else print deps
            const int modDirIdx = args.indexOf("--module-dir");
            if (modDirIdx != -1) {
                if (modDirIdx + 1 >= args.size()) {
                    err << "Usage: " << QFileInfo(app.applicationFilePath()).fileName() << " --metadata /path/to/metadata.json --module-dir /path/to/modules_dir\n";
                    return 1;
                }
                QString moduleDirArg = args.at(modDirIdx + 1);
                if (moduleDirArg.startsWith('@')) {
                    moduleDirArg.remove(0, 1);
                }
                QDir moduleDir(moduleDirArg);
                if (!moduleDir.exists()) {
                    err << "Module directory does not exist: " << moduleDirArg << "\n";
                    return 2;
                }

                QString genDirPath = QDir::current().filePath("logos-cpp-sdk/cpp/generated");
                QDir().mkpath(genDirPath);
                if (!ensureCoreManagerWrapper(genDirPath, err)) {
                    return 9;
                }

                QString suffix;
#if defined(Q_OS_MACOS)
                suffix = ".dylib";
#elif defined(Q_OS_LINUX)
                suffix = ".so";
#elif defined(Q_OS_WIN)
                suffix = ".dll";
#else
                suffix = "";
#endif

                int overallStatus = 0;
                for (const QJsonValue& v : deps) {
                    if (!v.isString()) continue;
                    const QString depName = v.toString();
                    const QString pluginFileName = depName + "_plugin" + suffix;
                    const QString pluginPath = moduleDir.filePath(pluginFileName);
                    if (!QFileInfo::exists(pluginPath)) {
                        err << "Skipping: plugin not found for dependency '" << depName << "' at " << pluginPath << "\n";
                        continue;
                    }
                    out << "Running generator for dependency plugin: " << pluginPath << "\n";
                    const int st = generateFromPlugin(pluginPath, out, err);
                    if (st != 0) {
                        overallStatus = st; // remember last non-zero
                    }
                }
                if (overallStatus == 0) {
                    if (!writeUmbrellaHeader(genDirPath, err)) {
                        overallStatus = 7;
                    } else if (!writeUmbrellaSource(genDirPath, err)) {
                        overallStatus = 8;
                    }
                }
                return overallStatus;
            } else {
                for (const QJsonValue& v : deps) {
                    if (v.isString()) {
                        out << v.toString() << "\n";
                    }
                }
                out.flush();
                return 0;
            }
        }
    }

    if (app.arguments().size() < 2) {
        err << "Usage: " << QFileInfo(app.applicationFilePath()).fileName() << " /absolute/path/to/plugin\n";
        err << "   or:  " << QFileInfo(app.applicationFilePath()).fileName() << " --metadata /absolute/path/to/metadata.json\n";
        return 1;
    }

    QString argPath = app.arguments().at(1);
    return generateFromPlugin(argPath, out, err);
}
