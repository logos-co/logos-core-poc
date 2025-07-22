#include "ChatUIComponent.h"
#include "src/ChatWidget.h"

QWidget* ChatUIComponent::createWidget() {
    return new ChatWidget();
}

void ChatUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
