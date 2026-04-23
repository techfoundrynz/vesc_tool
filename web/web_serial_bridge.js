let serialPort = null;
let serialReader = null;
let serialWriter = null;
let serialKeepReading = true;
let serialReadPromise = null;

window.requestWebSerial = async (baudRate) => {
    try {
        if (!navigator.serial) {
            console.error("Web Serial not supported in this browser.");
            return;
        }
        serialPort = await navigator.serial.requestPort();
        await serialPort.open({ baudRate: baudRate || 115200 });
        serialKeepReading = true;
        serialReader = serialPort.readable.getReader();
        serialWriter = serialPort.writable.getWriter();
        console.log("Web Serial connected!");
        
        serialReadPromise = (async () => {
             while (serialPort.readable && serialKeepReading) {
                 try {
                     const { value, done } = await serialReader.read();
                     if (done) break;
                     if (value && window.Module && window.Module._web_serial_rx) {
                         const len = value.byteLength;
                         const ptr = window.Module._malloc(len);
                         window.Module.HEAPU8.set(value, ptr);
                         window.Module._web_serial_rx(ptr, len, 0);
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
        await serialWriter.write(dataView);
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
