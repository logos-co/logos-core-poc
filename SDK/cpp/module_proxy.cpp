#include "module_proxy.h"
#include <QDebug>

ModuleProxy::ModuleProxy(QObject* module, QObject* parent)
    : QObject(parent)
    , m_module(module)
{
    qDebug() << "ModuleProxy: Created for module:" << module;
}

ModuleProxy::~ModuleProxy()
{
    qDebug() << "ModuleProxy: Destroyed for module:" << m_module;
}

// Include MOC for template instantiation
#include "moc_module_proxy.cpp" 