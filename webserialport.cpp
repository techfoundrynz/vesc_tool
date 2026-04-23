#include "webserialport.h"
#include <QDebug>

#ifdef Q_OS_WASM
#include <emscripten.h>
#include <emscripten/val.h>
#endif

WebSerialPort::WebSerialPort(QObject *parent) : QObject(parent)
{
    mIsOpen = false;
}

WebSerialPort::~WebSerialPort()
{
    close();
}

void WebSerialPort::setPortName(const QString &name)
{
    mName = name;
}

void WebSerialPort::setBaudRate(int )
{
}

bool WebSerialPort::open(int )
{
#ifdef Q_OS_WASM
    mIsOpen = false; // Will be set to true by web_serial_connected
    // We can call Emscripten JS to request port here
    EM_ASM({
        if (window.requestWebSerial) {
            window.requestWebSerial($0);
        }
    }, this);
    return true;
#else
    return false;
#endif
}

QByteArray WebSerialPort::readAll()
{
    QByteArray res = mBuffer;
    mBuffer.clear();
    return res;
}

void WebSerialPort::write(const QByteArray &data)
{
#ifdef Q_OS_WASM
    if(mIsOpen) {
        // Send data over Web Serial using EM_ASM instead of emscripten::val
        // to avoid dependency on --bind and hasOwnProperty quirks
        EM_ASM({
            console.log("C++ EM_ASM write triggered: ", $1, " bytes");
            if (window.writeWebSerial) {
                var view = new Uint8Array(window.Module.HEAPU8.buffer, $0, $1);
                window.writeWebSerial(view);
            } else {
                console.error("writeWebSerial is not present on window object!");
            }
        }, data.data(), data.size());
    }
#else
    (void)data;
#endif
}

void WebSerialPort::close()
{
    mIsOpen = false;
#ifdef Q_OS_WASM
    EM_ASM({
        if (window.closeWebSerial) {
            window.closeWebSerial();
        }
    });
#endif
}

bool WebSerialPort::isOpen() const
{
    return mIsOpen;
}

void WebSerialPort::clear()
{
    mBuffer.clear();
}

#ifdef Q_OS_WASM
void WebSerialPort::jsDataCallback(const char *data, int len, void *context)
{
    WebSerialPort *port = static_cast<WebSerialPort*>(context);
    port->mBuffer.append(data, len);
    emit port->readyRead();
}

void WebSerialPort::jsConnectedCallback(void *context)
{
    WebSerialPort *port = static_cast<WebSerialPort*>(context);
    port->mIsOpen = true;
    emit port->connected();
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void web_serial_rx(const char *data, int len, void *context) {
        WebSerialPort::jsDataCallback(data, len, context);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void web_serial_connected(void *context) {
        WebSerialPort::jsConnectedCallback(context);
    }
}
#endif
