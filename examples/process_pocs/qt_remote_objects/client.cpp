#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QRemoteObjectNode>
#include <QTimer>

class CounterWindow : public QMainWindow
{
    Q_OBJECT

public:
    CounterWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Remote Counter Client");
        resize(400, 200);

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
        
        statusLabel = new QLabel("Connecting to counter service...", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(countDisplay);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(statusLabel);
        
        setCentralWidget(centralWidget);
        
        // Set up Remote Objects
        remoteNode = new QRemoteObjectNode(this);
        remoteNode->connectToNode(QUrl(QStringLiteral("local:registry")));
        
        // Get the counter from the registry
        counter = remoteNode->acquireDynamic("Counter");
        
        // Connect signals and slots
        connect(counter, SIGNAL(countChanged()), this, SLOT(updateCount()));
        connect(counter, SIGNAL(stateChanged(QRemoteObjectReplica::State,QRemoteObjectReplica::State)), 
                this, SLOT(updateState()));
        connect(incrementButton, &QPushButton::clicked, this, &CounterWindow::incrementCounter);
        connect(decrementButton, &QPushButton::clicked, this, &CounterWindow::decrementCounter);
        connect(resetButton, &QPushButton::clicked, this, &CounterWindow::resetCounter);
        
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
        }
    }
    
    void incrementCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            QMetaObject::invokeMethod(counter, "increment");
        }
    }
    
    void decrementCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            QMetaObject::invokeMethod(counter, "decrement");
        }
    }
    
    void resetCounter() {
        if (counter && counter->state() == QRemoteObjectReplica::Valid) {
            QMetaObject::invokeMethod(counter, "reset");
        }
    }
    
private:
    QLabel *countDisplay;
    QLabel *statusLabel;
    QRemoteObjectNode *remoteNode;
    QRemoteObjectDynamicReplica *counter;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    CounterWindow window;
    window.show();
    
    return app.exec();
}

#include "client.moc" 