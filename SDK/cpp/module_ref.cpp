#include "module_ref.h"
#include "logos_api_client.h"

QObject* ModuleRef::requestObject(int timeoutMs) const {
  return m_client->requestObject(m_moduleName, timeoutMs);
}

void ModuleRef::on(const QString& eventName,
                   std::function<void(const QString&, const QVariantList&)> callback) const {
  QObject* origin = requestObject();
  if (!origin) {
    return;
  }
  // destinationObject not needed here; we route callback through the client
  m_client->onEvent(origin, nullptr, eventName, std::move(callback));
}


