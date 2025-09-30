#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonArray>
#include <QtCore/QStringList>
#include "../../logos-liblogos/interface.h"

class LogosIRCInterface : public PluginInterface
{
public:
    virtual ~LogosIRCInterface() {}
    Q_INVOKABLE virtual bool foo(const QString &bar) = 0;

signals:
    // for now this is required for events, later it might not be necessary if using a proxy
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define LogosIRCInterface_iid "org.logos.LogosIRCInterface"
Q_DECLARE_INTERFACE(LogosIRCInterface, LogosIRCInterface_iid) 