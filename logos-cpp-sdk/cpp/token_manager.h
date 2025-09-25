#ifndef TOKEN_MANAGER_H
#define TOKEN_MANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QMutex>

/**
 * @brief TokenManager provides a singleton interface for managing authentication tokens
 * 
 * This class manages a collection of tokens identified by keys, providing thread-safe
 * access to store, retrieve, and manage tokens throughout the application lifecycle.
 */
class TokenManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance of TokenManager
     * @return TokenManager& Reference to the singleton instance
     */
    static TokenManager& instance();

    /**
     * @brief Save a token with the given key
     * @param key The identifier for the token
     * @param token The token value to store
     */
    void saveToken(const QString& key, const QString& token);

    /**
     * @brief Retrieve a token by key
     * @param key The identifier for the token
     * @return QString The token value, or empty string if not found
     */
    QString getToken(const QString& key) const;

    /**
     * @brief Check if a token exists for the given key
     * @param key The identifier to check
     * @return bool True if token exists, false otherwise
     */
    bool hasToken(const QString& key) const;

    /**
     * @brief Remove a token by key
     * @param key The identifier for the token to remove
     * @return bool True if token was removed, false if it didn't exist
     */
    bool removeToken(const QString& key);

    /**
     * @brief Clear all tokens
     */
    void clearAllTokens();

    /**
     * @brief Get all token keys
     * @return QList<QString> List of all token keys
     */
    QList<QString> getTokenKeys() const;

    /**
     * @brief Get the number of stored tokens
     * @return int Number of tokens stored
     */
    int tokenCount() const;

signals:
    /**
     * @brief Emitted when a token is saved
     * @param key The key of the saved token
     */
    void tokenSaved(const QString& key);

    /**
     * @brief Emitted when a token is removed
     * @param key The key of the removed token
     */
    void tokenRemoved(const QString& key);

    /**
     * @brief Emitted when all tokens are cleared
     */
    void allTokensCleared();

private:
    /**
     * @brief Private constructor for singleton pattern
     * @param parent Parent QObject
     */
    explicit TokenManager(QObject *parent = nullptr);

    /**
     * @brief Private destructor
     */
    ~TokenManager();

    // Delete copy constructor and assignment operator to enforce singleton
    TokenManager(const TokenManager&) = delete;
    TokenManager& operator=(const TokenManager&) = delete;

    /**
     * @brief Hash map storing tokens by key
     */
    QHash<QString, QString> m_tokens;

    /**
     * @brief Mutex for thread-safe access to tokens
     */
    mutable QMutex m_mutex;
};

#endif // TOKEN_MANAGER_H 