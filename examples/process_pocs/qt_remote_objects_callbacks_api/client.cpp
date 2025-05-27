#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <QInputDialog>
#include <QTime>
#include "RemoteCounter.h"

class CounterWindow : public QMainWindow
{
    Q_OBJECT

public:
    CounterWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Remote Counter Client");
        resize(400, 300);

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
        
        // Add a manual refresh button for testing
        QPushButton *refreshButton = new QPushButton("Refresh UI", this);
        refreshButton->setMinimumSize(100, 40);
        
        resultLabel = new QLabel("Multiplication result: -", this);
        resultLabel->setAlignment(Qt::AlignCenter);
        
        statusLabel = new QLabel("Connecting to counter service...", this);
        statusLabel->setAlignment(Qt::AlignCenter);
        
        // Add an event display area for debugging
        QFrame *eventFrame = new QFrame(this);
        eventFrame->setFrameShape(QFrame::StyledPanel);
        eventFrame->setLineWidth(1);
        QVBoxLayout *eventLayout = new QVBoxLayout(eventFrame);
        eventDisplay = new QLabel("Event Log:", this);
        eventDisplay->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        eventDisplay->setWordWrap(true);
        eventLayout->addWidget(eventDisplay);
        
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(countDisplay);
        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(multiplyButton);
        mainLayout->addWidget(refreshButton);
        mainLayout->addWidget(resultLabel);
        mainLayout->addWidget(statusLabel);
        mainLayout->addWidget(eventFrame);
        
        setCentralWidget(centralWidget);
        
        // Create the counter object
        counter = new RemoteCounter(this);

        // Connect signals for UI updates - use both connection methods for redundancy
        // Direct connection for the SIGNAL/SLOT style
        connect(counter, SIGNAL(countChanged()), this, SLOT(updateCountDisplay()));
        
        // Also try with the new style connect
        connect(counter, &RemoteCounter::countChanged, this, [this]() {
            addEventLog("countChanged signal received via lambda");
            updateCountDisplay();
        });
        
        // Set up a timer to poll for updates in case signals aren't working
        QTimer *pollTimer = new QTimer(this);
        connect(pollTimer, &QTimer::timeout, this, [this]() {
            static int lastCount = -1;
            int currentCount = counter->count();
            if (currentCount != lastCount) {
                addEventLog(QString("Poll timer detected count change: %1 -> %2").arg(lastCount).arg(currentCount));
                lastCount = currentCount;
                updateCountDisplay();
            }
        });
        pollTimer->start(500); // Poll every 500ms

        connect(counter, &RemoteCounter::connectionStateChanged, this, [this](QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState) {
            addEventLog(QString("Connection state changed: %1 -> %2").arg(oldState).arg(state));
            
            if (state == QRemoteObjectReplica::Valid) {
                statusLabel->setText("Connected");
                statusLabel->setStyleSheet("color: green");
                
                // Force an update of the display when connected
                QTimer::singleShot(100, this, &CounterWindow::updateCountDisplay);
            } else {
                statusLabel->setText("Disconnected");
                statusLabel->setStyleSheet("color: red");
            }
        });

        // Button connections
        connect(incrementButton, &QPushButton::clicked, this, [this]() {
            addEventLog("Increment button clicked");
            counter->increment();
            
            // Force a UI update after a short delay
            QTimer::singleShot(100, this, &CounterWindow::updateCountDisplay);
        });
        
        connect(decrementButton, &QPushButton::clicked, this, [this]() {
            addEventLog("Decrement button clicked");
            counter->decrement();
            
            // Force a UI update after a short delay
            QTimer::singleShot(100, this, &CounterWindow::updateCountDisplay);
        });
        
        connect(resetButton, &QPushButton::clicked, this, [this]() {
            addEventLog("Reset button clicked");
            counter->reset();
            
            // Force a UI update after a short delay
            QTimer::singleShot(100, this, &CounterWindow::updateCountDisplay);
        });
        
        connect(refreshButton, &QPushButton::clicked, this, [this]() {
            addEventLog("Refresh button clicked");
            updateCountDisplay();
        });
        
        connect(multiplyButton, &QPushButton::clicked, this, &CounterWindow::onMultiplyButtonClicked);
        
        // Initialize the event log
        eventLogs.clear();
        addEventLog("Application started");
    }
    
private slots:
    void updateCountDisplay() {
        int currentCount = counter->count();
        qDebug() << "CounterWindow::updateCountDisplay - Setting display to:" << currentCount;
        addEventLog(QString("Updating display to: %1").arg(currentCount));
        countDisplay->setText(QString::number(currentCount));
    }
    
    void onMultiplyButtonClicked() {
        bool ok;
        int factor = QInputDialog::getInt(this, "Multiply", "Enter factor:", 2, -100, 100, 1, &ok);
        
        if (ok) {
            addEventLog(QString("Multiply button clicked with factor: %1").arg(factor));
            
            // Simple callback-based API - no request IDs or signals visible!
            counter->multiply(factor, [this, factor](int result) {
                addEventLog(QString("Multiply result received: %1").arg(result));
                
                resultLabel->setText(QString("Result: %1 × %2 = %3")
                                 .arg(result/factor)
                                 .arg(factor)
                                 .arg(result));
                resultLabel->setStyleSheet("color: blue; font-weight: bold;");
                QTimer::singleShot(2000, this, [this]() {
                    resultLabel->setStyleSheet("");
                });
                
                // Also update the counter display after multiplication result
                updateCountDisplay();
            });
        }
    }
    
private:
    void addEventLog(const QString &message) {
        // Add timestamp
        QString entry = QString("[%1] %2")
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
            .arg(message);
            
        eventLogs.prepend(entry);
        while (eventLogs.size() > 10) {
            eventLogs.removeLast();
        }
        
        eventDisplay->setText("Event Log:\n" + eventLogs.join("\n"));
    }
    
private:
    QLabel *countDisplay;
    QLabel *statusLabel;
    QLabel *resultLabel;
    QLabel *eventDisplay;
    QStringList eventLogs;
    RemoteCounter *counter;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    CounterWindow window;
    window.show();
    
    return app.exec();
}

#include "client.moc" 