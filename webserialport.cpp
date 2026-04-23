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
    mIsOpen = true;
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
        // Send data over Web Serial
        emscripten::val view = emscripten::val(emscripten::typed_memory_view(data.size(), (const uint8_t*)data.data()));
        emscripten::val window = emscripten::val::global("window");
        if (window.hasOwnProperty("writeWebSerial")) {
            window.call<void>("writeWebSerial", view);
        }
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

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void web_serial_rx(const char *data, int len, void *context) {
        WebSerialPort::jsDataCallback(data, len, context);
    }
}
#endif
