#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QRemoteObjectNode>
#include <QTimer>
#include <QInputDialog>
#include <QUuid>
#include <QMap>
#include <QDateTime>
#include <QScrollArea>
#include "Counter.h"

class CounterWindow : public QMainWindow
{
    Q_OBJECT

public:
    CounterWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Remote Counter Client");
        resize(400, 400);  // Increased height for pending requests

        // Create widgets
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        QLabel *titleLabel = new QLabel("Remote Counter Example", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        
        countDisplay = new QLabel("0", this);
        countDisplay->setAlignment(Qt::AlignCenter);
        QFont countFont = countDisplay->font();
        countFont.setPointSize(24);
        countFont.setBold(true);
        countDisplay->setFont(countFont);
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        
        QPushButton *decrementButton = new QPushButton("-", this);
        decrementButton->setMinimumSize(80, 40);
        QFont buttonFont = decrementButton->font();
        buttonFont.setPointSize(16);
        decrementButton->setFont(buttonFont);
        
        QPushButton *resetButton = new QPushButton("Reset", this);
        resetButton->setMinimumSize(80, 40);
        resetButton->setFont(buttonFont);
        
        QPushButton *incrementButton = new QPushButton("+", this);
        incrementButton->setMinimumSize(80, 40);
        incrementButton->setFont(buttonFont);
        
        buttonLayout->addWidget(decrementButton);
        buttonLayout->addWidget(resetButton);
        buttonLayout->addWidget(incrementButton);
        
        // Add multiply button
        QPushButton *multiplyButton = new QPushButton("Multiply", this);
        multiplyButton->setMinimumSize(100, 40);
        multiplyButton->setFont(buttonFont);
        
        resultLabel = new QLabel("Multiplication result: -", this);
        resultLabel->setAlignment(Qt::AlignCenter);
        
        // Add section for pending requests
        QLabel *pendingTitle = new QLabel("Pending Requests:", this);
        pendingTitle->setAlignment(Qt::AlignLeft);
        pendingTitle->setFont(titleFont);
        
        // Create layout for pending request labels
        QWidget *pendingWidget = new QWidget(this);
        pendingLayout = new QVBoxLayout(pendingWidget);
        pendingLayout->setAlignment(Qt::AlignTop);
        pendingWidget->setLayout(pendingLayout);
        
        // Add scroll area for pending requests
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(pendingWidget);
        scrollArea->setMinimumHeight(100);
        
        statusLabel = new QLabel("Connecting to counter service...", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(countDisplay);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(multiplyButton);
        mainLayout->addWidget(resultLabel);
        mainLayout->addWidget(pendingTitle);
        mainLayout->addWidget(scrollArea);
        mainLayout->addWidget(statusLabel);
        
        setCentralWidget(centralWidget);
        
        // Set up Remote Objects
        remoteNode = new QRemoteObjectNode(this);
        remoteNode->connectToNode(QUrl(QStringLiteral("local:registry")));
        
        // Get the counter from the registry - using template version to get typed object
        counter = remoteNode->acquireDynamic("Counter");
        typedCounter = nullptr;
        
        // Connect signals and slots
        connect(counter, SIGNAL(countChanged()), this, SLOT(updateCount()));
        connect(counter, SIGNAL(stateChanged(QRemoteObjectReplica::State,QRemoteObjectReplica::State)), 
                this, SLOT(updateState()));
        
        // Wait for counter to be valid before connecting the multiplyResult signal
        connect(counter, &QRemoteObjectDynamicReplica::initialized, this, [this]() {
            // Connect to the dynamic multiplyResult signal using string-based connection
            bool connected = connect(counter, SIGNAL(multiplyResult(QString,int)), 
                                    this, SLOT(showMultiplyResult(QString,int)));
            if (!connected) {
                qDebug() << "Failed to connect multiplyResult signal";
            } else {
                qDebug() << "Successfully connected multiplyResult signal";
            }
            
            // Try to cast the dynamic replica to a Counter object
            typedCounter = qobject_cast<Counter*>(counter);
            if (!typedCounter) {
                qDebug() << "Failed to cast counter to Counter class";
            } else {
                qDebug() << "Successfully cast counter to Counter class";
            }
        });
        
        connect(incrementButton, &QPushButton::clicked, this, &CounterWindow::incrementCounter);
        connect(decrementButton, &QPushButton::clicked, this, &CounterWindow::decrementCounter);
        connect(resetButton, &QPushButton::clicked, this, &CounterWindow::resetCounter);
        connect(multiplyButton, &QPushButton::clicked, this, &CounterWindow::multiplyCounter);
        
        // Check connection status every second until connected
        QTimer *connectionTimer = new QTimer(this);
        connect(connectionTimer, &QTimer::timeout, this, &CounterWindow::checkConnection);
        connectionTimer->start(1000);
    }
    
private slots:
    void updateCount() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            int count = counter->property("count").toInt();
            countDisplay->setText(QString::number(count));
        }
    }
    
    void updateState() {
        if (counter) {
            if (counter->state() == QRemoteObjectReplica::Valid) {
                statusLabel->setText("Connected to counter service");
                statusLabel->setStyleSheet("color: green");
                updateCount();
            } else if (counter->state() == QRemoteObjectReplica::Suspect) {
                statusLabel->setText("Connection unstable");
                statusLabel->setStyleSheet("color: orange");
            } else {
                statusLabel->setText("Disconnected from counter service");
                statusLabel->setStyleSheet("color: red");
            }
        }
    }
    
    void checkConnection() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            updateCount();
            updateState();
            
            // Try to cast if not already done
            if (!typedCounter) {
                typedCounter = qobject_cast<Counter*>(counter);
                if (typedCounter) {
                    qDebug() << "Successfully cast counter to Counter class during connection check";
                }
            }
        }
    }
    
    void incrementCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            if (typedCounter) {
                // Use the strongly typed interface
                typedCounter->increment();
                qDebug() << "Called increment via typed interface";
            } else {
                // Fallback to invokeMethod
                QMetaObject::invokeMethod(counter, "increment");
                qDebug() << "Called increment via QMetaObject::invokeMethod";
            }
        }
    }
    
    void decrementCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            if (typedCounter) {
                // Use the strongly typed interface
                typedCounter->decrement();
                qDebug() << "Called decrement via typed interface";
            } else {
                // Fallback to invokeMethod
                QMetaObject::invokeMethod(counter, "decrement");
                qDebug() << "Called decrement via QMetaObject::invokeMethod";
            }
        }
    }
    
    void resetCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            if (typedCounter) {
                // Use the strongly typed interface
                typedCounter->reset();
                qDebug() << "Called reset via typed interface";
            } else {
                // Fallback to invokeMethod
                QMetaObject::invokeMethod(counter, "reset");
                qDebug() << "Called reset via QMetaObject::invokeMethod";
            }
        }
    }
    
    void multiplyCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            bool ok;
            int factor = QInputDialog::getInt(this, "Multiply Counter", 
                                        "Enter multiplication factor:", 2, -100, 100, 1, &ok);
            if (ok) {
                // Generate a unique ID for this request
                QString requestId = QUuid::createUuid().toString();
                
                // Store the pending request with timestamp for potential timeout handling
                pendingRequests[requestId] = PendingRequest{factor, QDateTime::currentDateTime()};
                
                // Create a specific label for this request
                QLabel* requestLabel = new QLabel("Request " + requestId.left(8) + "... (factor: " + 
                                               QString::number(factor) + "): calculating...", this);
                requestLabel->setAlignment(Qt::AlignLeft);
                
                // Add to the pending requests UI
                pendingLayout->addWidget(requestLabel);
                pendingLabels[requestId] = requestLabel;
                
                if (typedCounter) {
                    // Use the strongly typed interface
                    typedCounter->multiplyWithCallback(requestId, factor);
                    qDebug() << "Called multiplyWithCallback via typed interface";
                } else {
                    // Fallback to invokeMethod
                    QMetaObject::invokeMethod(counter, "multiplyWithCallback", 
                                            Q_ARG(QString, requestId),
                                            Q_ARG(int, factor));
                    qDebug() << "Called multiplyWithCallback via QMetaObject::invokeMethod";
                }
                
                qDebug() << "Sent multiply request with ID:" << requestId << "factor:" << factor;
            }
        }
    }
    
    void showMultiplyResult(const QString &requestId, int result) {
        qDebug() << "Received multiply result for request:" << requestId << "result:" << result;
        
        // Check if we have this request in our pending map
        if (pendingRequests.contains(requestId)) {
            int factor = pendingRequests[requestId].factor;
            
            // Update the request label with the result
            if (pendingLabels.contains(requestId)) {
                QLabel* label = pendingLabels[requestId];
                label->setText("Request " + requestId.left(8) + "... (factor: " + 
                            QString::number(factor) + "): result = " + QString::number(result));
                label->setStyleSheet("color: green;");
                
                // Schedule cleanup after some time
                QTimer::singleShot(10000, this, [this, requestId]() {
                    if (pendingLabels.contains(requestId)) {
                        QLabel* label = pendingLabels.take(requestId);
                        pendingLayout->removeWidget(label);
                        delete label;
                        pendingRequests.remove(requestId);
                    }
                });
            }
            
            // Also update the main result label
            resultLabel->setText(QString("Last result: %1 × %2 = %3")
                              .arg(result/factor)
                              .arg(factor)
                              .arg(result));
            resultLabel->setStyleSheet("color: blue; font-weight: bold;");
            QTimer::singleShot(2000, this, [this]() { 
                resultLabel->setStyleSheet(""); 
            });
        }
    }
    
private:
    QRemoteObjectNode *remoteNode;
    QRemoteObjectDynamicReplica *counter;
    Counter *typedCounter;  // The cast version of counter
    QLabel *countDisplay;
    QLabel *resultLabel;
    QLabel *statusLabel;
    QVBoxLayout *pendingLayout;
    
    struct PendingRequest {
        int factor;
        QDateTime timestamp;
    };
    
    QMap<QString, PendingRequest> pendingRequests;
    QMap<QString, QLabel*> pendingLabels;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    CounterWindow window;
    window.show();
    
    return app.exec();
}

#include "client.moc" 