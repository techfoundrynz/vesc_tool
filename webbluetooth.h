#ifndef WEBBLUETOOTH_H
#define WEBBLUETOOTH_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

class WebBluetooth : public QObject
{
    Q_OBJECT
public:
    explicit WebBluetooth(QObject *parent = nullptr);
    ~WebBluetooth();

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void startConnect(QString addr);
    Q_INVOKABLE void disconnectBle();
    Q_INVOKABLE bool isConnected();
    Q_INVOKABLE bool isConnecting();
    Q_INVOKABLE void emitScanDone();

signals:
    void dataRx(QByteArray data);
    void scanDone(QVariantMap devs, bool done);
    void bleError(QString info);
    void connected();
    void unintentionalDisconnect();

public slots:
    void writeData(QByteArray data);

private:
    bool mIsConnected;
public:
#ifdef Q_OS_WASM
    static void jsDataCallback(const char *data, int len, void *context);
    static void jsScanCallback(const char *addr, const char *name, void *context);
    static void jsConnectedCallback(void *context);
    static void jsDisconnectedCallback(void *context);
#endif
};

#endif // WEBBLUETOOTH_H
