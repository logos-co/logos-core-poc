#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDebug>
#include <QTimer>
#include "include/RemoteCounter.h"

class AbstractCallbackDemo : public QWidget {
    Q_OBJECT

public:
    AbstractCallbackDemo(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        
        m_counter = new RemoteCounter(this);
        
        // Connect signals
        connect(m_counter, &RemoteCounter::countChanged, this, &AbstractCallbackDemo::updateCount);
        connect(m_counter, &RemoteCounter::initialized, this, [this]() {
            m_statusLabel->setText("Status: Connected");
            updateCount();
        });
        
        // Auto-refresh count
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &AbstractCallbackDemo::updateCount);
        timer->start(1000);
    }

private slots:
    void updateCount() {
        m_countLabel->setText(QString("Count: %1").arg(m_counter->count()));
    }
    
    void onAddOperation() {
        int a = m_addSpinBox1->value();
        int b = m_addSpinBox2->value();
        
        m_resultTextEdit->append(QString("Executing ADD operation: %1 + %2 + count").arg(a).arg(b));
        
        // Using the generic callback
        m_counter->executeOperation("add", QVariantList{a, b}, 
            [this](const QVariantList& results) {
                if (results.size() >= 3) {
                    m_resultTextEdit->append(QString("ADD Result: %1, Message: %2, Time: %3")
                        .arg(results[0].toInt())
                        .arg(results[1].toString())
                        .arg(results[2].toDateTime().toString()));
                }
            });
    }
    
    void onMultiplyAdvanced() {
        int factor = m_multiplySpinBox->value();
        double multiplier = m_multiplierSpinBox->value();
        
        m_resultTextEdit->append(QString("Executing MULTIPLY_ADVANCED: count * %1 * %2").arg(factor).arg(multiplier));
        
        // Using the type-safe template callback
        m_counter->executeOperation("multiply_advanced", QVariantList{factor, multiplier},
            [this](double result, int factor, double multiplier, int originalCount, QString message) {
                m_resultTextEdit->append(QString("MULTIPLY_ADVANCED Result: %1 (was %2 * %3 * %4), Message: %5")
                    .arg(result)
                    .arg(originalCount)
                    .arg(factor)
                    .arg(multiplier)
                    .arg(message));
            });
    }
    
    void onStatistics() {
        // Create a list of numbers for statistics
        QVariantList numbers;
        numbers << 10 << 20 << 15 << 8 << 25 << 30;
        
        m_resultTextEdit->append("Executing STATISTICS operation on: [10, 20, 15, 8, 25, 30]");
        
        // Using type-safe callback with multiple return values
        m_counter->executeOperation("statistics", QVariantList{numbers},
            [this](double sum, double avg, double min, double max, int count, QString message) {
                m_resultTextEdit->append(QString("STATISTICS Result: Sum=%1, Avg=%2, Min=%3, Max=%4, Count=%5, %6")
                    .arg(sum)
                    .arg(avg)
                    .arg(min)
                    .arg(max)
                    .arg(count)
                    .arg(message));
            });
    }
    
    void onPowerOperation() {
        int exponent = m_powerSpinBox->value();
        
        m_resultTextEdit->append(QString("Executing POWER: count^%1").arg(exponent));
        
        m_counter->executeOperation("power", QVariantList{exponent},
            [this](double result, int exponent, int originalCount, QString message) {
                m_resultTextEdit->append(QString("POWER Result: %1^%2 = %3, %4")
                    .arg(originalCount)
                    .arg(exponent)
                    .arg(result)
                    .arg(message));
            });
    }
    
    void onStringOperation() {
        QString text1 = m_stringEdit1->text();
        QString text2 = m_stringEdit2->text();
        
        m_resultTextEdit->append(QString("Executing STRING operation with: '%1', '%2'").arg(text1).arg(text2));
        
        m_counter->executeOperation("string_operation", QVariantList{text1, text2},
            [this](QString result, int argCount, int currentCount) {
                m_resultTextEdit->append(QString("STRING Result: '%1' (processed %2 args, count was %3)")
                    .arg(result)
                    .arg(argCount)
                    .arg(currentCount));
            });
    }
    
    void clearResults() {
        m_resultTextEdit->clear();
    }

private:
    void setupUI() {
        setWindowTitle("Abstract Callback Demo");
        setMinimumSize(800, 600);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // Status section
        QGroupBox *statusGroup = new QGroupBox("Status", this);
        QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
        m_statusLabel = new QLabel("Status: Connecting...", statusGroup);
        m_countLabel = new QLabel("Count: 0", statusGroup);
        statusLayout->addWidget(m_statusLabel);
        statusLayout->addWidget(m_countLabel);
        mainLayout->addWidget(statusGroup);
        
        // Basic operations
        QGroupBox *basicGroup = new QGroupBox("Basic Operations", this);
        QHBoxLayout *basicLayout = new QHBoxLayout(basicGroup);
        
        QPushButton *incBtn = new QPushButton("Increment", this);
        QPushButton *decBtn = new QPushButton("Decrement", this);
        QPushButton *resetBtn = new QPushButton("Reset", this);
        
        connect(incBtn, &QPushButton::clicked, m_counter, &RemoteCounter::increment);
        connect(decBtn, &QPushButton::clicked, m_counter, &RemoteCounter::decrement);
        connect(resetBtn, &QPushButton::clicked, m_counter, &RemoteCounter::reset);
        
        basicLayout->addWidget(incBtn);
        basicLayout->addWidget(decBtn);
        basicLayout->addWidget(resetBtn);
        mainLayout->addWidget(basicGroup);
        
        // Abstract operations
        QGroupBox *abstractGroup = new QGroupBox("Abstract Operations", this);
        QVBoxLayout *abstractLayout = new QVBoxLayout(abstractGroup);
        
        // Add operation
        QHBoxLayout *addLayout = new QHBoxLayout();
        addLayout->addWidget(new QLabel("Add:"));
        m_addSpinBox1 = new QSpinBox(this);
        m_addSpinBox1->setRange(-1000, 1000);
        m_addSpinBox1->setValue(5);
        m_addSpinBox2 = new QSpinBox(this);
        m_addSpinBox2->setRange(-1000, 1000);
        m_addSpinBox2->setValue(10);
        addLayout->addWidget(m_addSpinBox1);
        addLayout->addWidget(new QLabel("+"));
        addLayout->addWidget(m_addSpinBox2);
        addLayout->addWidget(new QLabel("+ count"));
        QPushButton *addBtn = new QPushButton("Execute Add", this);
        connect(addBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::onAddOperation);
        addLayout->addWidget(addBtn);
        abstractLayout->addLayout(addLayout);
        
        // Multiply advanced operation
        QHBoxLayout *multiplyLayout = new QHBoxLayout();
        multiplyLayout->addWidget(new QLabel("Multiply Advanced:"));
        m_multiplySpinBox = new QSpinBox(this);
        m_multiplySpinBox->setRange(1, 100);
        m_multiplySpinBox->setValue(3);
        m_multiplierSpinBox = new QDoubleSpinBox(this);
        m_multiplierSpinBox->setRange(0.1, 10.0);
        m_multiplierSpinBox->setSingleStep(0.1);
        m_multiplierSpinBox->setValue(2.5);
        multiplyLayout->addWidget(new QLabel("count *"));
        multiplyLayout->addWidget(m_multiplySpinBox);
        multiplyLayout->addWidget(new QLabel("*"));
        multiplyLayout->addWidget(m_multiplierSpinBox);
        QPushButton *multiplyBtn = new QPushButton("Execute Multiply", this);
        connect(multiplyBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::onMultiplyAdvanced);
        multiplyLayout->addWidget(multiplyBtn);
        abstractLayout->addLayout(multiplyLayout);
        
        // Statistics operation
        QHBoxLayout *statsLayout = new QHBoxLayout();
        statsLayout->addWidget(new QLabel("Statistics on fixed array [10,20,15,8,25,30]:"));
        QPushButton *statsBtn = new QPushButton("Execute Statistics", this);
        connect(statsBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::onStatistics);
        statsLayout->addWidget(statsBtn);
        abstractLayout->addLayout(statsLayout);
        
        // Power operation
        QHBoxLayout *powerLayout = new QHBoxLayout();
        powerLayout->addWidget(new QLabel("Power: count^"));
        m_powerSpinBox = new QSpinBox(this);
        m_powerSpinBox->setRange(1, 10);
        m_powerSpinBox->setValue(2);
        powerLayout->addWidget(m_powerSpinBox);
        QPushButton *powerBtn = new QPushButton("Execute Power", this);
        connect(powerBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::onPowerOperation);
        powerLayout->addWidget(powerBtn);
        abstractLayout->addLayout(powerLayout);
        
        // String operation
        QHBoxLayout *stringLayout = new QHBoxLayout();
        stringLayout->addWidget(new QLabel("String Operation:"));
        m_stringEdit1 = new QLineEdit("Hello", this);
        m_stringEdit2 = new QLineEdit("World", this);
        stringLayout->addWidget(m_stringEdit1);
        stringLayout->addWidget(m_stringEdit2);
        QPushButton *stringBtn = new QPushButton("Execute String", this);
        connect(stringBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::onStringOperation);
        stringLayout->addWidget(stringBtn);
        abstractLayout->addLayout(stringLayout);
        
        mainLayout->addWidget(abstractGroup);
        
        // Results section
        QGroupBox *resultsGroup = new QGroupBox("Results", this);
        QVBoxLayout *resultsLayout = new QVBoxLayout(resultsGroup);
        m_resultTextEdit = new QTextEdit(this);
        m_resultTextEdit->setReadOnly(true);
        QPushButton *clearBtn = new QPushButton("Clear Results", this);
        connect(clearBtn, &QPushButton::clicked, this, &AbstractCallbackDemo::clearResults);
        resultsLayout->addWidget(m_resultTextEdit);
        resultsLayout->addWidget(clearBtn);
        mainLayout->addWidget(resultsGroup);
    }

private:
    RemoteCounter *m_counter;
    QLabel *m_statusLabel;
    QLabel *m_countLabel;
    QTextEdit *m_resultTextEdit;
    
    // UI controls for operations
    QSpinBox *m_addSpinBox1;
    QSpinBox *m_addSpinBox2;
    QSpinBox *m_multiplySpinBox;
    QDoubleSpinBox *m_multiplierSpinBox;
    QSpinBox *m_powerSpinBox;
    QLineEdit *m_stringEdit1;
    QLineEdit *m_stringEdit2;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    AbstractCallbackDemo demo;
    demo.show();
    
    return app.exec();
}

#include "client_demo_abstract.moc" 