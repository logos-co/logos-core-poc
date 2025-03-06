#pragma once

#include <QObject>
#include <QWidget>
#include <QtPlugin>

class IComponent {
public:
    virtual ~IComponent() = default;
    virtual QWidget* createWidget() = 0;
    virtual void destroyWidget(QWidget* widget) = 0;
};

#define IComponent_iid "com.logos.component.IComponent"
Q_DECLARE_INTERFACE(IComponent, IComponent_iid) 