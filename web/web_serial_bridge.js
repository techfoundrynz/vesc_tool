let serialPort = null;
let serialReader = null;
let serialWriter = null;
let serialKeepReading = true;
let serialReadPromise = null;

window.requestWebSerial = async (contextPtr) => {
    try {
        if (!navigator.serial) {
            console.error("Web Serial not supported in this browser.");
            return;
        }
        serialPort = await navigator.serial.requestPort();
        await serialPort.open({ baudRate: 115200 });
        serialKeepReading = true;
        serialReader = serialPort.readable.getReader();
        serialWriter = serialPort.writable.getWriter();
        console.log("Web Serial connected! Setting DTR/RTS...");
        // Some USB CDC devices require DTR/RTS asserted to begin streaming
        try {
            await serialPort.setSignals({ dataTerminalReady: true, requestToSend: true });
            console.log("Signals set successfully.");
        } catch (e) {
            console.warn("Failed to set DTR/RTS signals (normal for some adapters):", e);
        }
        
        if (window.Module && window.Module._web_serial_connected) {
            window.Module._web_serial_connected(contextPtr);
        }
        
        serialReadPromise = (async () => {
             console.log("Starting Web Serial read loop...");
             while (serialPort.readable && serialKeepReading) {
                 try {
                     const { value, done } = await serialReader.read();
                     if (done) break;
                     if (value && window.Module && window.Module._web_serial_rx) {
                         console.log("RX: " + value.byteLength + " bytes");
                         const len = value.byteLength;
                         const ptr = window.Module._malloc(len);
                         window.Module.HEAPU8.set(value, ptr);
                         window.Module._web_serial_rx(ptr, len, contextPtr);
                         window.Module._free(ptr);
                     }
                 } catch (e) {
                     console.error("Serial read error", e);
                 }
             }
         })();
    } catch (err) {
        console.error("Web Serial request failed", err);
    }
};

window.writeWebSerial = async (dataView) => {
    if (serialWriter) {
        // CRITICAL: We must copy the data out of the WASM heap view
        // because the async write might happen after C++ frees the QByteArray
        const copy = new Uint8Array(dataView);
        console.log("TX: " + copy.byteLength + " bytes");
        try {
            await serialWriter.write(copy);
        } catch (e) {
            console.error("Serial write failed:", e);
        }
    }
};

window.closeWebSerial = async () => {
    serialKeepReading = false;
    if (serialReader) {
        await serialReader.cancel();
        serialReader.releaseLock();
        serialReader = null;
    }
    if (serialWriter) {
        serialWriter.releaseLock();
        serialWriter = null;
    }
    if (serialReadPromise) {
        await serialReadPromise;
    }
    if (serialPort) {
        await serialPort.close();
        serialPort = null;
    }
    console.log("Web Serial closed");
};
