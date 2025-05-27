#include <QtCore/QCoreApplication>
#include <QtCore>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QUuid>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtRemoteObjects>
#include "rep_loggerinterface_source.h"

class Logger : public LoggerInterfaceSource
{
    Q_OBJECT

public:
    Logger(const QString &guid, QObject *parent = nullptr) 
        : LoggerInterfaceSource(parent), m_guid(guid)
    {
        // Initialize the guid property
    }

    // Implement pure virtual methods
    QString guid() const override
    {
        return m_guid;
    }
    
    void setGuid(QString guid) override
    {
        if (m_guid != guid) {
            m_guid = guid;
            emit guidChanged(m_guid);
        }
    }

public slots:
    void logMessage(const QString &message) override
    {
        QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
        qDebug() << "Message received from process_host_counter:" << message << "at" << timestamp;
        
        // Emit signal to notify listeners
        emit messageLogged(message, timestamp);
    }

private:
    QString m_guid;
};

class ProcessHandler : public QObject
{
    Q_OBJECT

public:
    ProcessHandler(QRemoteObjectHost *host, QObject *parent = nullptr) : QObject(parent), m_host(host)
    {
        // Generate a random GUID
        m_guid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        qDebug() << "Generated GUID:" << m_guid;
        
        // Create and expose the logger source
        m_logger = new Logger(m_guid, this);
        m_host->enableRemoting(m_logger, "LoggerInterface");
        qDebug() << "Exposed LoggerInterface on remote object host";
        
        // Connect to the logger signals
        connect(m_logger, &LoggerInterfaceSource::messageLogged, 
                this, &ProcessHandler::onMessageLogged);
        
        // Create the counter process
        m_counterProcess = new QProcess(this);
        
        // Connect to counter process signals
        connect(m_counterProcess, &QProcess::finished, this, &ProcessHandler::onCounterProcessFinished);
        connect(m_counterProcess, &QProcess::errorOccurred, this, &ProcessHandler::onCounterProcessError);
        connect(m_counterProcess, &QProcess::readyReadStandardOutput, this, &ProcessHandler::onCounterProcessOutput);
        
        // Create the calculator process
        m_calculatorProcess = new QProcess(this);
        
        // Connect to calculator process signals
        connect(m_calculatorProcess, &QProcess::finished, this, &ProcessHandler::onCalculatorProcessFinished);
        connect(m_calculatorProcess, &QProcess::errorOccurred, this, &ProcessHandler::onCalculatorProcessError);
        connect(m_calculatorProcess, &QProcess::readyReadStandardOutput, this, &ProcessHandler::onCalculatorProcessOutput);
        
        // Start the processes
        startCounterProcess();
        startCalculatorProcess();
        
        // Set up a timer to monitor resource usage
        m_monitorTimer = new QTimer(this);
        connect(m_monitorTimer, &QTimer::timeout, this, &ProcessHandler::monitorResourceUsage);
        m_monitorTimer->start(5000); // Check every 5 seconds
    }
    
    ~ProcessHandler()
    {
        // Terminate the counter process when parent exits
        if (m_counterProcess->state() == QProcess::Running) {
            qDebug() << "Terminating process_host_counter...";
            m_counterProcess->terminate();
            m_counterProcess->waitForFinished(3000); // Wait up to 3 seconds
            
            // If it didn't terminate gracefully, kill it
            if (m_counterProcess->state() == QProcess::Running) {
                qDebug() << "Counter process didn't terminate gracefully, killing it...";
                m_counterProcess->kill();
            }
        }
        
        // Terminate the calculator process when parent exits
        if (m_calculatorProcess->state() == QProcess::Running) {
            qDebug() << "Terminating process_host_calculator...";
            m_calculatorProcess->terminate();
            m_calculatorProcess->waitForFinished(3000); // Wait up to 3 seconds
            
            // If it didn't terminate gracefully, kill it
            if (m_calculatorProcess->state() == QProcess::Running) {
                qDebug() << "Calculator process didn't terminate gracefully, killing it...";
                m_calculatorProcess->kill();
            }
        }
    }

private slots:
    void onCounterProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        qDebug() << "Counter process finished with exit code:" << exitCode;
    }
    
    void onCounterProcessError(QProcess::ProcessError error)
    {
        qDebug() << "Counter process error:" << error;
    }
    
    void onCounterProcessOutput()
    {
        QByteArray output = m_counterProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            qDebug() << "Counter process output:" << output;
        }
    }
    
    void onCalculatorProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        qDebug() << "Calculator process finished with exit code:" << exitCode;
    }
    
    void onCalculatorProcessError(QProcess::ProcessError error)
    {
        qDebug() << "Calculator process error:" << error;
    }
    
    void onCalculatorProcessOutput()
    {
        QByteArray output = m_calculatorProcess->readAllStandardOutput();
        if (!output.isEmpty()) {
            qDebug() << "Calculator process output:" << output;
        }
    }
    
    void monitorResourceUsage()
    {
        // Monitor counter process
        if (m_counterProcess->state() == QProcess::Running) {
            monitorProcessResources(m_counterProcess->processId(), "Counter");
        }
        
        // Monitor calculator process
        if (m_calculatorProcess->state() == QProcess::Running) {
            monitorProcessResources(m_calculatorProcess->processId(), "Calculator");
        }
    }
    
    void onMessageLogged(const QString &message, const QString &timestamp)
    {
        qDebug() << "Logger event - Message:" << message << "Timestamp:" << timestamp;
    }

private:
    void monitorProcessResources(qint64 pid, const QString &processName)
    {
        // On macOS, use ps command to get CPU and memory usage
        QProcess ps;
        ps.start("ps", QStringList() << "-p" << QString::number(pid) << "-o" << "pid,%cpu,%mem,rss,vsz");
        
        if (ps.waitForFinished(1000)) {
            QString output = QString::fromUtf8(ps.readAllStandardOutput());
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            
            if (lines.size() >= 2) {
                QString header = lines[0].trimmed();
                QString values = lines[1].trimmed();
                
                qDebug() << "Resource usage for" << processName << "process (PID:" << pid << "):";
                qDebug() << "  " << header;
                qDebug() << "  " << values;
            } else {
                qDebug() << "Could not get resource information for" << processName << "process (PID:" << pid << ")";
            }
        } else {
            qDebug() << "Failed to run ps command:" << ps.errorString();
        }
    }
    
    void startCounterProcess()
    {
        // Find the process_host_counter executable using a relative path
        QString processPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../counter/build/process_host_counter");
        
        qDebug() << "Starting process_host_counter:" << processPath;
        
        // Set up the process to capture output
        m_counterProcess->setProcessChannelMode(QProcess::MergedChannels);
        
        // Set up arguments to pass the GUID and remote object URL
        QStringList arguments;
        arguments << "--guid" << m_guid;
        arguments << "--registry" << m_host->hostUrl().toString();
        
        // Start the process with arguments
        qDebug() << "Passing GUID to process_host_counter:" << m_guid;
        qDebug() << "Passing Registry URL:" << m_host->hostUrl().toString();
        m_counterProcess->start(processPath, arguments);
        
        if (m_counterProcess->waitForStarted()) {
            qDebug() << "Counter process started successfully with PID:" << m_counterProcess->processId();
        } else {
            qDebug() << "Failed to start process_host_counter:" << m_counterProcess->errorString();
        }
    }
    
    void startCalculatorProcess()
    {
        // Find the calculator executable using a relative path
        QString processPath = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../calculator/build/process_host_calculator");
        
        qDebug() << "Starting process_host_calculator:" << processPath;
        
        // Set up the process to capture output
        m_calculatorProcess->setProcessChannelMode(QProcess::MergedChannels);
        
        // Set up arguments to pass the registry URL
        QStringList arguments;
        arguments << "--registry" << m_host->hostUrl().toString();
        
        // Start the process with arguments
        qDebug() << "Passing Registry URL to calculator:" << m_host->hostUrl().toString();
        m_calculatorProcess->start(processPath, arguments);
        
        if (m_calculatorProcess->waitForStarted()) {
            qDebug() << "Calculator process started successfully with PID:" << m_calculatorProcess->processId();
        } else {
            qDebug() << "Failed to start process_host_calculator:" << m_calculatorProcess->errorString();
        }
    }

private:
    QProcess* m_counterProcess;
    QProcess* m_calculatorProcess;
    QString m_guid;
    QTimer* m_monitorTimer;
    QRemoteObjectHost* m_host;
    Logger* m_logger;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set up QRemoteObjectHost
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:registry")));
    qDebug() << "Remote object host started at:" << srcNode.hostUrl();
    
    ProcessHandler handler(&srcNode);
    
    return app.exec();
}

#include "main.moc"