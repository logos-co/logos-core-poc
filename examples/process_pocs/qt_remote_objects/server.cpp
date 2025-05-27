#include <QCoreApplication>
#include <QRemoteObjectHost>
#include <QDebug>

// A simple counter class
class Counter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)

public:
    Counter(QObject *parent = nullptr) : QObject(parent), m_count(0) {
        qDebug() << "Counter created with initial count:" << m_count;
    }

    int count() const { return m_count; }
    void setCount(int count) {
        if (m_count != count) {
            m_count = count;
            emit countChanged();
        }
    }

public slots:
    void increment() {
        setCount(m_count + 1);
        qDebug() << "Counter incremented to:" << m_count;
    }

    void decrement() {
        setCount(m_count - 1);
        qDebug() << "Counter decremented to:" << m_count;
    }

    void reset() {
        setCount(0);
        qDebug() << "Counter reset to:" << m_count;
    }

signals:
    void countChanged();

private:
    int m_count;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Create a host node to share objects
    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:registry")));
    
    // Create and share the counter
    Counter *counter = new Counter(&srcNode);
    srcNode.enableRemoting(counter, "Counter");
    
    qDebug() << "Counter Server started. Registry available at:" << "local:registry";
    qDebug() << "Press Ctrl+C to quit";

    return app.exec();
}

#include "server.moc" 