#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QJsonArray>
#include <QStringList>
#include <functional>

#include "logos_api_client.h"

/**
 * ModuleRef is a lightweight, header-only helper that provides an ergonomic
 * interface for calling remote methods and subscribing to events on a target
 * module. It wraps a `LogosAPIClient*` and the module name.
 */
class ModuleRef {
public:
  ModuleRef(LogosAPIClient* client, const QString& moduleName)
      : m_client(client), m_moduleName(moduleName) {}

  const QString& moduleName() const { return m_moduleName; }

  // Generic call building a QVariantList from variadic arguments (default timeout)
  template <typename... Args>
  QVariant callVariant(const QString& methodName, Args&&... args) const {
    QVariantList params;
    appendAll(params, std::forward<Args>(args)...);
    return m_client->invokeRemoteMethod(m_moduleName, methodName, params, 20000);
  }

  // Same as above but with explicit timeout override
  template <typename... Args>
  QVariant callVariantWithTimeout(const QString& methodName, int timeoutMs, Args&&... args) const {
    QVariantList params;
    appendAll(params, std::forward<Args>(args)...);
    return m_client->invokeRemoteMethod(m_moduleName, methodName, params, timeoutMs);
  }

  // Typed helpers
  template <typename... Args>
  bool callBool(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    return v.toBool();
  }

  template <typename... Args>
  QString callString(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    return v.toString();
  }

  template <typename... Args>
  int callInt(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    return v.toInt();
  }

  template <typename... Args>
  double callDouble(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    return v.toDouble();
  }

  template <typename... Args>
  QJsonArray callJsonArray(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    if (v.canConvert<QJsonArray>()) {
      return v.toJsonArray();
    }
    return QJsonArray();
  }

  template <typename... Args>
  QStringList callStringList(const QString& methodName, int timeoutMs = 20000, Args&&... args) const {
    QVariant v = callVariantWithTimeout(methodName, timeoutMs, std::forward<Args>(args)...);
    if (v.canConvert<QStringList>()) {
      return v.toStringList();
    }
    return QStringList();
  }

  // Event subscription using a callback; acquires a replica internally
  void on(const QString& eventName,
          std::function<void(const QString&, const QVariantList&)> callback) const;

  // Access to the raw replica for advanced scenarios
  QObject* requestObject(int timeoutMs = 20000) const;

private:
  // Base case: no arguments
  static void appendAll(QVariantList&) {}

  // Recursive variadic append
  template <typename T, typename... Rest>
  static void appendAll(QVariantList& list, T&& value, Rest&&... rest) {
    list << QVariant::fromValue(std::forward<T>(value));
    appendAll(list, std::forward<Rest>(rest)...);
  }

private:
  LogosAPIClient* m_client;
  QString m_moduleName;
};


