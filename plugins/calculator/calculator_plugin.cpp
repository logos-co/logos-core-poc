#include "calculator_plugin.h"
#include <QDebug>

CalculatorPlugin::CalculatorPlugin(QObject *parent)
    : QObject(parent)
    , m_name("calculator")
    , m_version("1.0.0")
    , m_timer(new QTimer(this))
{
    qDebug() << "Calculator Plugin initialized!";
    
    // Connect the timer to the printCalculatorMessage slot
    connect(m_timer, &QTimer::timeout, this, &CalculatorPlugin::printCalculatorMessage);
}

CalculatorPlugin::~CalculatorPlugin()
{
    // Stop the timer if it's running
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    
    qDebug() << "Calculator Plugin destroyed!";
}

int CalculatorPlugin::add(int a, int b)
{
    return a + b;
}

int CalculatorPlugin::subtract(int a, int b)
{
    return a - b;
}

QString CalculatorPlugin::echo(const QString &message)
{
    // Store the message
    m_message = message;

    // Start the timer to print the message every 2 seconds (2000 ms)
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    m_timer->start(3000);

    QString result = "Calculator: " + message;
    qDebug() << result;
    qDebug() << "Calculator timer started. Will print message every 2 seconds.";

    return result;
}

void CalculatorPlugin::printCalculatorMessage()
{
    qDebug() << "Calculator timer message: " << m_message;
}

QString CalculatorPlugin::name() const
{
    return m_name;
}

QString CalculatorPlugin::version() const
{
    return m_version;
} 