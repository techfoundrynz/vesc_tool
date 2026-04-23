#include "webbluetooth.h"
#include <QDebug>

#ifdef Q_OS_WASM
#include <emscripten.h>
#include <emscripten/val.h>
#endif

WebBluetooth::WebBluetooth(QObject *parent) : QObject(parent)
{
    mIsConnected = false;
}

WebBluetooth::~WebBluetooth()
{
    disconnectBle();
}

void WebBluetooth::startScan()
{
#ifdef Q_OS_WASM
    EM_ASM({
        if (window.webBleScan) {
            window.webBleScan($0);
        }
    }, this);
#endif
}

void WebBluetooth::startConnect(QString )
{
    // The web browser typically forces you to scan AND connect in one shot (requestDevice).
    // So webBleScan handles the pairing. We don't really use startConnect directly with an address in WASM.
}

void WebBluetooth::disconnectBle()
{
    mIsConnected = false;
#ifdef Q_OS_WASM
    EM_ASM({
        if (window.webBleDisconnect) {
            window.webBleDisconnect();
        }
    });
#endif
}

bool WebBluetooth::isConnected()
{
    return mIsConnected;
}

bool WebBluetooth::isConnecting()
{
    return false;
}

void WebBluetooth::emitScanDone()
{
    QVariantMap devs;
    emit scanDone(devs, true);
}

void WebBluetooth::writeData(QByteArray data)
{
#ifdef Q_OS_WASM
    if(mIsConnected) {
        emscripten::val view = emscripten::val(emscripten::typed_memory_view(data.size(), (const uint8_t*)data.data()));
        emscripten::val window = emscripten::val::global("window");
        if (window.hasOwnProperty("webBleWrite")) {
            window.call<void>("webBleWrite", view);
        }
    }
#else
    (void)data;
#endif
}

#ifdef Q_OS_WASM
void WebBluetooth::jsDataCallback(const char *data, int len, void *context)
{
    WebBluetooth *ble = static_cast<WebBluetooth*>(context);
    emit ble->dataRx(QByteArray(data, len));
}

void WebBluetooth::jsScanCallback(const char *addr, const char *name, void *context)
{
    WebBluetooth *ble = static_cast<WebBluetooth*>(context);
    QVariantMap devs;
    devs.insert(QString(addr), QString(name));
    emit ble->scanDone(devs, true);
}

void WebBluetooth::jsConnectedCallback(void *context)
{
    WebBluetooth *ble = static_cast<WebBluetooth*>(context);
    ble->mIsConnected = true;
    emit ble->connected();
}

void WebBluetooth::jsDisconnectedCallback(void *context)
{
    WebBluetooth *ble = static_cast<WebBluetooth*>(context);
    ble->mIsConnected = false;
    emit ble->unintentionalDisconnect();
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void web_ble_rx(const char *data, int len, void *context) {
        WebBluetooth::jsDataCallback(data, len, context);
    }
    EMSCRIPTEN_KEEPALIVE
    void web_ble_scan_res(const char *addr, const char *name, void *context) {
        WebBluetooth::jsScanCallback(addr, name, context);
    }
    EMSCRIPTEN_KEEPALIVE
    void web_ble_connected(void *context) {
        WebBluetooth::jsConnectedCallback(context);
    }
    EMSCRIPTEN_KEEPALIVE
    void web_ble_disconnected(void *context) {
        WebBluetooth::jsDisconnectedCallback(context);
    }
}
#endif
