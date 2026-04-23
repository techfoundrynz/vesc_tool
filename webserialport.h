#ifndef WEBSERIALPORT_H
#define WEBSERIALPORT_H

#include <QObject>
#include <QByteArray>

class WebSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit WebSerialPort(QObject *parent = nullptr);
    ~WebSerialPort();

    void setPortName(const QString &name);
    QString portName() const { return mName; }
    void setBaudRate(int baudRate);
    bool open(int mode = 0);
    QByteArray readAll();
    void write(const QByteArray &data);
    void close();
    bool isOpen() const;
    void clear();
    qint64 bytesAvailable() const { return mBuffer.size(); }

signals:
    void readyRead();
    void error(int err);

private:
    bool mIsOpen;
    QString mName;
    QByteArray mBuffer;
public:
#ifdef Q_OS_WASM
    static void jsDataCallback(const char *data, int len, void *context);
#endif
};

#endif // WEBSERIALPORT_H
