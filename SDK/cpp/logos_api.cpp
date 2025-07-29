#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_api_provider.h"
#include "token_manager.h"

LogosAPI::LogosAPI(const QString& module_name, QObject *parent)
    : QObject(parent)
    , m_module_name(module_name)
    , m_provider(nullptr)
    , m_token_manager(nullptr)
{
    // Initialize provider
    m_provider = new LogosAPIProvider(m_module_name, this);
    
    // Get token manager instance
    m_token_manager = &TokenManager::instance();
}

LogosAPI::~LogosAPI()
{
    // Provider and client will be automatically deleted as child objects
    // Token manager is a singleton, so we don't delete it
}

LogosAPIProvider* LogosAPI::getProvider() const
{
    return m_provider;
}

LogosAPIClient* LogosAPI::getClient(const QString& target_module) const
{
    // Check if we already have a client for this target module
    if (m_clients.contains(target_module)) {
        return m_clients.value(target_module);
    }
    
    // Create a new client for this target module
    LogosAPIClient* client = new LogosAPIClient(target_module, m_module_name, m_token_manager, const_cast<LogosAPI*>(this));
    
    // Cache it for future use
    m_clients.insert(target_module, client);
    
    return client;
}

TokenManager* LogosAPI::getTokenManager() const
{
    return m_token_manager;
} 
