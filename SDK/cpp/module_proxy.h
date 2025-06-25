#ifndef MODULE_PROXY_H
#define MODULE_PROXY_H

#include <QObject>

/**
 * @brief ModuleProxy provides a proxy interface for module interactions
 * 
 * This class serves as a proxy layer for communicating with modules
 * in the Logos Core system.
 */
class ModuleProxy : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new ModuleProxy
     * @param module The module object to proxy
     * @param parent Parent QObject
     */
    explicit ModuleProxy(QObject* module, QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ModuleProxy();

private:
    QObject* m_module;
};

#endif // MODULE_PROXY_H 