/**
 * @file simple_example.cpp
 * @brief Simple example showing how to use the LogosAPI class
 */

#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_api_provider.h"
#include "token_manager.h"
#include <QDebug>

void simpleExample()
{
    // Create a LogosAPI instance for our module
    LogosAPI api("core");
    
    // Get the provider and register an object
    LogosAPIProvider* provider = api.getProvider();
    QObject* myService = new QObject();
    provider->registerObject("my_service", myService);
    
    // Get the client to communicate with other modules
    LogosAPIClient* client = api.getClient("core_manager");
    // Use client to call remote methods...
    
    // Get the token manager and save tokens
    TokenManager* tokenManager = api.getTokenManager();
    tokenManager->saveToken("auth_token", "abc123");
    
    qDebug() << "LogosAPI initialized successfully!";
    
    delete myService;
} 