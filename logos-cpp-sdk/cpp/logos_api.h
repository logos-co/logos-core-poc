#ifndef LOGOS_API_H
#define LOGOS_API_H

#include <QObject>
#include <QString>
#include <QHash>

class LogosAPIClient;
class LogosAPIProvider;
class TokenManager;

/**
 * @brief LogosAPI provides a unified interface to the Logos SDK
 * 
 * This class initializes and keeps instances of the client provider and token manager.
 */
class LogosAPI : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new LogosAPI instance
     * @param module_name The name of this module
     * @param parent Parent QObject
     */
    explicit LogosAPI(const QString& module_name, QObject *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~LogosAPI();

    /**
     * @brief Get the client provider instance
     * @return LogosAPIProvider* Pointer to the provider
     */
    LogosAPIProvider* getProvider() const;

    /**
     * @brief Get the client instance for communicating with a module
     * @param target_module The module to communicate with
     * @return LogosAPIClient* Pointer to the client
     */
    LogosAPIClient* getClient(const QString& target_module) const;

    /**
     * @brief Get the token manager instance
     * @return TokenManager* Pointer to the token manager
     */
    TokenManager* getTokenManager() const;

private:
    QString m_module_name;
    LogosAPIProvider* m_provider;
    mutable QHash<QString, LogosAPIClient*> m_clients;  // Cache of clients per target module
    TokenManager* m_token_manager;
};

#endif // LOGOS_API_H 