#ifndef MODULE_PROXY_H
#define MODULE_PROXY_H

#include <QObject>

class ModuleProxy : public QObject
{
    Q_OBJECT

public:
    explicit ModuleProxy(QObject *parent = nullptr);
};

#endif // MODULE_PROXY_H 