#include "token_manager.h"
#include <QMutexLocker>

TokenManager& TokenManager::instance()
{
    static TokenManager instance;
    return instance;
}

TokenManager::TokenManager(QObject *parent)
    : QObject(parent)
{
}

TokenManager::~TokenManager()
{
}

void TokenManager::saveToken(const QString& key, const QString& token)
{
    QMutexLocker locker(&m_mutex);
    m_tokens[key] = token;
    emit tokenSaved(key);
}

QString TokenManager::getToken(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    return m_tokens.value(key, QString());
}

bool TokenManager::hasToken(const QString& key) const
{
    QMutexLocker locker(&m_mutex);
    return m_tokens.contains(key);
}

bool TokenManager::removeToken(const QString& key)
{
    QMutexLocker locker(&m_mutex);
    if (m_tokens.contains(key)) {
        m_tokens.remove(key);
        emit tokenRemoved(key);
        return true;
    }
    return false;
}

void TokenManager::clearAllTokens()
{
    QMutexLocker locker(&m_mutex);
    m_tokens.clear();
    emit allTokensCleared();
}

QList<QString> TokenManager::getTokenKeys() const
{
    QMutexLocker locker(&m_mutex);
    return m_tokens.keys();
}

int TokenManager::tokenCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_tokens.size();
} 