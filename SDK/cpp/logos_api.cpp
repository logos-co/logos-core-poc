#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_api_provider.h"
#include "token_manager.h"

LogosAPI::LogosAPI(const QString& module_name, QObject *parent)
    : QObject(parent)
    , m_module_name(module_name)
    , m_provider(nullptr)
    , m_client(nullptr)
    , m_token_manager(nullptr)
{
    // Initialize provider
    m_provider = new LogosAPIProvider(m_module_name, this);
    
    // Initialize client (with default target - can be changed later)
    m_client = new LogosAPIClient("core_manager", m_module_name, this);
    
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
    // For simplicity, return the existing client
    // In a more complex implementation, you might create clients per module
    Q_UNUSED(target_module)
    return m_client;
}

TokenManager* LogosAPI::getTokenManager() const
{
    return m_token_manager;
} 
