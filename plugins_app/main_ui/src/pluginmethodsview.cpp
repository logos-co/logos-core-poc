#include "pluginmethodsview.h"
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include "core/plugin_registry.h"

PluginMethodsView::PluginMethodsView(const QString& pluginName, QWidget* parent)
    : QWidget(parent)
    , m_pluginName(pluginName)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
    , m_methodsTree(nullptr)
    , m_backButton(nullptr)
    , m_generateInterfaceButton(nullptr)
{
    setupUi();
    loadPluginMethods();
}

PluginMethodsView::~PluginMethodsView()
{
    // Qt automatically cleans up child widgets
}

void PluginMethodsView::setupUi()
{
    // Set up the layout
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(20);
    m_layout->setContentsMargins(40, 40, 40, 40);

    // Create a header layout with back button and title
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    // Create back button
    m_backButton = new QPushButton("← Back", this);
    m_backButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4b4b4b;"
        "   color: #ffffff;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5b5b5b;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3b3b3b;"
        "}"
    );
    connect(m_backButton, &QPushButton::clicked, this, &PluginMethodsView::onBackButtonClicked);
    headerLayout->addWidget(m_backButton);
    
    // Add horizontal spacer after back button
    headerLayout->addSpacing(20);

    // Create and style the title
    m_titleLabel = new QLabel(m_pluginName + " Methods", this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #ffffff;");
    headerLayout->addWidget(m_titleLabel);
    
    // Add stretch to push everything to the left
    headerLayout->addStretch();
    
    // Create Generate Interface button
    m_generateInterfaceButton = new QPushButton("Generate Interface", this);
    m_generateInterfaceButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #2a82da;"
        "   color: #ffffff;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3a92ea;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1a72ca;"
        "}"
    );
    connect(m_generateInterfaceButton, &QPushButton::clicked, this, &PluginMethodsView::onGenerateInterfaceButtonClicked);
    headerLayout->addWidget(m_generateInterfaceButton);
    
    // Add the header layout to main layout
    m_layout->addLayout(headerLayout);

    // Create tree widget for displaying methods
    m_methodsTree = new QTreeWidget(this);
    m_methodsTree->setColumnCount(4);
    m_methodsTree->setHeaderLabels(QStringList() << "Method" << "Return Type" << "Parameters" << "Invokable");
    m_methodsTree->setAlternatingRowColors(true);
    m_methodsTree->setAnimated(true);
    m_methodsTree->setSortingEnabled(true);
    m_methodsTree->sortByColumn(0, Qt::AscendingOrder);
    m_methodsTree->setStyleSheet(
        "QTreeWidget {"
        "   background-color: #2d2d2d;"
        "   alternate-background-color: #353535;"
        "   color: #e0e0e0;"
        "   border: 1px solid #3d3d3d;"
        "   border-radius: 4px;"
        "}"
        "QTreeWidget::item {"
        "   padding: 4px;"
        "}"
        "QTreeWidget::item:selected {"
        "   background-color: #2a82da;"
        "}"
        "QHeaderView::section {"
        "   background-color: #4b4b4b;"
        "   color: #ffffff;"
        "   padding: 8px;"
        "   border: none;"
        "}"
    );
    m_methodsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_methodsTree->header()->setStretchLastSection(false);
    m_methodsTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    // Connect double-click signal
    connect(m_methodsTree, &QTreeWidget::itemDoubleClicked, this, &PluginMethodsView::onMethodItemDoubleClicked);

    // Add widgets to layout
    m_layout->addWidget(m_methodsTree);

    // Set style for the view
    setStyleSheet("background-color: #1e1e1e;");
}

void PluginMethodsView::loadPluginMethods()
{
    // Get the core manager from the registry
    QObject* coreManagerObj = PluginRegistry::getPlugin<QObject>("core_manager");
    if (!coreManagerObj) {
        qWarning() << "CoreManager plugin not found";
        m_methodsTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << "Error: CoreManager plugin not found"));
        return;
    }
    
    // Check available methods on the plugin
    const QMetaObject* metaObj = coreManagerObj->metaObject();
    qDebug() << "Available methods on CoreManager:";
    for (int i = 0; i < metaObj->methodCount(); ++i) {
        QMetaMethod method = metaObj->method(i);
        qDebug() << " - " << method.methodSignature();
    }
    
    // Try to find the specific getPluginMethods method
    int methodIndex = metaObj->indexOfMethod("getPluginMethods(QString)");
    if (methodIndex == -1) {
        qWarning() << "getPluginMethods method not found on CoreManager plugin";
        m_methodsTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << "Error: getPluginMethods method not found"));
        return;
    }
    
    // Invoke the method once we found it
    QJsonArray methods;
    QMetaMethod method = metaObj->method(methodIndex);
    bool success = method.invoke(
        coreManagerObj,
        Q_RETURN_ARG(QJsonArray, methods),
        Q_ARG(QString, m_pluginName)
    );
    
    if (!success) {
        qWarning() << "Failed to invoke getPluginMethods method";
        m_methodsTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << "Error: Failed to invoke getPluginMethods method"));
        return;
    }
    
    // Display the methods in the tree widget
    displayPluginMethods(methods);
}

void PluginMethodsView::displayPluginMethods(const QJsonArray& methods)
{
    m_methodsTree->clear();
    
    if (methods.isEmpty()) {
        m_methodsTree->addTopLevelItem(new QTreeWidgetItem(QStringList() << "No methods found for this plugin"));
        return;
    }
    
    for (const QJsonValue& methodValue : methods) {
        QJsonObject methodObj = methodValue.toObject();
        
        // Get method details
        QString signature = methodObj["signature"].toString();
        QString name = methodObj["name"].toString();
        QString returnType = methodObj["returnType"].toString();
        bool isInvokable = methodObj["isInvokable"].toBool();
        
        // Create tree item for the method
        QTreeWidgetItem* methodItem = new QTreeWidgetItem();
        methodItem->setText(0, name);
        methodItem->setText(1, returnType.isEmpty() ? "void" : returnType);
        
        // Process parameters
        QJsonArray params = methodObj["parameters"].toArray();
        QStringList paramStrings;
        
        for (const QJsonValue& paramValue : params) {
            QJsonObject paramObj = paramValue.toObject();
            QString paramType = paramObj["type"].toString();
            QString paramName = paramObj["name"].toString();
            paramStrings << paramType + " " + paramName;
        }
        
        methodItem->setText(2, paramStrings.join(", "));
        methodItem->setText(3, isInvokable ? "Yes" : "No");
        
        // Store the full method information in the item data
        methodItem->setData(0, Qt::UserRole, methodObj.toVariantMap());
        
        // Add the method to the tree
        m_methodsTree->addTopLevelItem(methodItem);
    }
    
    // Expand all items
    m_methodsTree->expandAll();
}

void PluginMethodsView::onBackButtonClicked()
{
    emit backClicked();
}

void PluginMethodsView::onMethodItemDoubleClicked(QTreeWidgetItem* item, int column)
{
    if (!item) return;
    
    // Get the method information from the item data
    QVariantMap methodData = item->data(0, Qt::UserRole).toMap();
    if (methodData.isEmpty()) return;
    
    // Display method details in a message box
    QString methodDetails = QString("<b>Method:</b> %1<br>").arg(methodData["signature"].toString());
    methodDetails += QString("<b>Return Type:</b> %1<br>").arg(methodData["returnType"].toString().isEmpty() ? "void" : methodData["returnType"].toString());
    methodDetails += "<b>Parameters:</b><br>";
    
    QVariantList params = methodData["parameters"].toList();
    if (params.isEmpty()) {
        methodDetails += "None<br>";
    } else {
        for (const QVariant& param : params) {
            QVariantMap paramMap = param.toMap();
            methodDetails += QString("&nbsp;&nbsp;- %1 %2<br>").arg(paramMap["type"].toString()).arg(paramMap["name"].toString());
        }
    }
    
    methodDetails += QString("<b>Invokable:</b> %1").arg(methodData["isInvokable"].toBool() ? "Yes" : "No");
    
    QMessageBox::information(this, "Method Details", methodDetails);
}

void PluginMethodsView::onGenerateInterfaceButtonClicked()
{
    // Get plugin methods
    QObject* coreManagerObj = PluginRegistry::getPlugin<QObject>("core_manager");
    if (!coreManagerObj) {
        QMessageBox::warning(this, "Error", "CoreManager plugin not found");
        return;
    }
    
    int methodIndex = coreManagerObj->metaObject()->indexOfMethod("getPluginMethods(QString)");
    if (methodIndex == -1) {
        QMessageBox::warning(this, "Error", "getPluginMethods method not found");
        return;
    }
    
    QJsonArray methods;
    QMetaMethod method = coreManagerObj->metaObject()->method(methodIndex);
    bool success = method.invoke(
        coreManagerObj,
        Q_RETURN_ARG(QJsonArray, methods),
        Q_ARG(QString, m_pluginName)
    );
    
    if (!success || methods.isEmpty()) {
        QMessageBox::warning(this, "Error", "Failed to retrieve plugin methods or no methods found");
        return;
    }
    
    // Generate interface file content
    QString interfaceName = m_pluginName + "Interface";
    QString guardName = interfaceName.toUpper() + "_H";
    QString interfaceId = "com.logos." + interfaceName;
    
    QString interfaceContent;
    QTextStream stream(&interfaceContent);
    
    // Start with include guards and headers
    stream << "#ifndef " << guardName << "\n";
    stream << "#define " << guardName << "\n\n";
    
    stream << "#include <QtCore/QObject>\n";
    stream << "#include <QtPlugin>\n";
    stream << "#include <QString>\n\n";
    
    // Add PluginInterface definition with an ifdef check
    stream << "#ifndef PLUGIN_INTERFACE_H\n";
    stream << "#define PLUGIN_INTERFACE_H\n\n";
    
    stream << "// Define the common base interface for all plugins\n";
    stream << "class PluginInterface\n";
    stream << "{\n";
    stream << "public:\n";
    stream << "    virtual ~PluginInterface() {}\n";
    stream << "    \n";
    stream << "    // Common plugin methods\n";
    stream << "    virtual QString name() const = 0;\n";
    stream << "    virtual QString version() const = 0;\n";
    stream << "};\n\n";
    
    stream << "// Define the interface ID used by Qt's plugin system\n";
    stream << "#define PluginInterface_iid \"com.logos.PluginInterface\"\n\n";
    
    stream << "Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)\n\n";
    
    stream << "#endif // PLUGIN_INTERFACE_H\n\n";
    
    // Add callback type definitions if there are any
    QSet<QString> callbackTypes;
    for (const QJsonValue& methodValue : methods) {
        QJsonObject methodObj = methodValue.toObject();
        QString name = methodObj["name"].toString();
        QJsonArray params = methodObj["parameters"].toArray();
        
        for (const QJsonValue& paramValue : params) {
            QJsonObject paramObj = paramValue.toObject();
            QString paramType = paramObj["type"].toString();
            
            // Check if parameter type looks like a callback (contains "Callback" or "std::function")
            if (paramType.contains("Callback") || paramType.contains("std::function")) {
                // Extract the callback type if not already defined
                if (!callbackTypes.contains(paramType)) {
                    callbackTypes.insert(paramType);
                }
            }
        }
    }
    
    // Add callback type definitions if any found
    if (!callbackTypes.isEmpty()) {
        stream << "// Callback type definitions\n";
        for (const QString& callbackType : callbackTypes) {
            // If the type is already fully qualified (contains std::function), use it directly
            if (callbackType.contains("std::function")) {
                QString typeName = callbackType.split(" ").first();
                stream << "using " << typeName << " = " << callbackType << ";\n";
            }
        }
        stream << "\n";
    }
    
    // Class declaration
    stream << "class " << interfaceName << " : public PluginInterface {\n";
    stream << "public:\n";
    stream << "    virtual ~" << interfaceName << "() {}\n\n";
    stream << "    // Plugin methods\n";
    
    // Add method declarations
    for (const QJsonValue& methodValue : methods) {
        QJsonObject methodObj = methodValue.toObject();
        QString name = methodObj["name"].toString();
        QString returnType = methodObj["returnType"].toString();
        if (returnType.isEmpty()) {
            returnType = "void";
        }
        
        // Build parameter list
        QStringList paramList;
        QJsonArray params = methodObj["parameters"].toArray();
        
        for (const QJsonValue& paramValue : params) {
            QJsonObject paramObj = paramValue.toObject();
            QString paramType = paramObj["type"].toString();
            QString paramName = paramObj["name"].toString();
            
            // Add default value for callback parameters
            if (paramType.contains("Callback") || paramType.contains("std::function")) {
                paramList << paramType + " " + paramName + " = nullptr";
            } else {
                paramList << paramType + " " + paramName;
            }
        }
        
        // Add the method declaration
        stream << "    virtual " << returnType << " " << name << "(" << paramList.join(", ") << ") = 0;\n";
    }
    
    // Close class declaration
    stream << "};\n\n";
    
    // Add interface ID
    stream << "#define " << interfaceName << "_iid \"" << interfaceId << "\"\n";
    stream << "Q_DECLARE_INTERFACE(" << interfaceName << ", " << interfaceName << "_iid)\n\n";
    
    // Close include guard
    stream << "#endif // " << guardName << "\n";
    
    // Create interfaces directory if it doesn't exist
    QDir interfacesDir("interfaces");
    if (!interfacesDir.exists()) {
        QDir().mkdir("interfaces");
    }
    
    // Save the file
    QString filePath = "interfaces/" + m_pluginName.toLower() + ".h";
    QFile file(filePath);
    
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream outStream(&file);
        outStream << interfaceContent;
        file.close();
        
        QMessageBox::information(this, "Interface Generated", 
            "Interface file has been generated successfully.\n\n"
            "File: " + QDir::currentPath() + "/" + filePath);
    } else {
        QMessageBox::warning(this, "Error", "Failed to create interface file: " + file.errorString());
    }
}