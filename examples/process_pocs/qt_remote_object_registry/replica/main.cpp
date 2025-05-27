#include <QApplication>
#include <QRemoteObjectNode>
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "rep_counter_replica.h"

class CounterWindow : public QWidget
{
    Q_OBJECT
public:
    CounterWindow(QSharedPointer<CounterReplica> counter, QWidget *parent = nullptr)
        : QWidget(parent), m_counter(counter)
    {
        // Set up the UI
        setWindowTitle("Remote Counter Client");
        resize(300, 200);
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        
        m_valueLabel = new QLabel("Value: 0", this);
        m_valueLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_valueLabel);
        
        QPushButton *incrementBtn = new QPushButton("Increment", this);
        layout->addWidget(incrementBtn);
        
        QPushButton *decrementBtn = new QPushButton("Decrement", this);
        layout->addWidget(decrementBtn);
        
        // Connect signals and slots
        connect(incrementBtn, &QPushButton::clicked, m_counter.data(), &CounterReplica::increment);
        connect(decrementBtn, &QPushButton::clicked, m_counter.data(), &CounterReplica::decrement);
        
        connect(m_counter.data(), &CounterReplica::valueChanged, this, &CounterWindow::updateValue);
        connect(m_counter.data(), &CounterReplica::initialized, this, &CounterWindow::onInitialized);
        
        // Show initial state if already initialized
        if (m_counter->isInitialized()) {
            updateValue(m_counter->value());
        }
    }

private slots:
    void updateValue(int value) {
        m_valueLabel->setText(QString("Value: %1").arg(value));
    }
    
    void onInitialized() {
        qDebug() << "Counter replica initialized";
        updateValue(m_counter->value());
    }

private:
    QSharedPointer<CounterReplica> m_counter;
    QLabel *m_valueLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Connect to the registry
    QRemoteObjectNode replicaNode;
    replicaNode.connectToNode(QUrl("tcp://localhost:45000"));
    
    // Acquire the counter
    QSharedPointer<CounterReplica> counter(replicaNode.acquire<CounterReplica>("Counter"));
    
    if (!counter) {
        qDebug() << "Failed to acquire Counter from registry";
        return 1;
    }
    
    qDebug() << "Connected to registry, waiting for Counter object...";
    
    // Create and show the window
    CounterWindow window(counter);
    window.show();
    
    return app.exec();
}

#include "main.moc" 