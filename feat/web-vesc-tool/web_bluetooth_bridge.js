let bleDevice = null;
let bleServer = null;
let bleService = null;
let bleRxChar = null;
let bleTxChar = null;

// VESC UART UUIDs
const SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

window.webBleScan = async () => {
    try {
        if (!navigator.bluetooth) {
            console.error("Web Bluetooth not supported in this browser.");
            return;
        }
        bleDevice = await navigator.bluetooth.requestDevice({
            filters: [{ services: [SERVICE_UUID] }],
            optionalServices: [SERVICE_UUID] // Sometimes required depending on Chrome versions
        });

        bleDevice.addEventListener('gattserverdisconnected', () => {
            console.log("BLE Disconnected");
            if (window.Module && window.Module._web_ble_disconnected) {
                window.Module._web_ble_disconnected(0);
            }
        });

        // WebBLE requires connecting to access services
        bleServer = await bleDevice.gatt.connect();
        bleService = await bleServer.getPrimaryService(SERVICE_UUID);
        
        // Characteristic retrieval might vary per device. Typical Nordic UART: RX=0002, TX=0003
        bleRxChar = await bleService.getCharacteristic(TX_UUID); // device transmits to us (RX)
        bleTxChar = await bleService.getCharacteristic(RX_UUID); // device receives from us (TX)

        if (window.Module && window.Module._web_ble_scan_res) {
            // Fake scan result for VescInterface UI
            const idBytes = new TextEncoder().encode(bleDevice.id);
            const nameBytes = new TextEncoder().encode(bleDevice.name || "VESC");
            
            const idPtr = window.Module._malloc(idBytes.length + 1);
            const namePtr = window.Module._malloc(nameBytes.length + 1);
            
            window.Module.HEAPU8.set(idBytes, idPtr);
            window.Module.HEAPU8[idPtr + idBytes.length] = 0; // null terminator
            window.Module.HEAPU8.set(nameBytes, namePtr);
            window.Module.HEAPU8[namePtr + nameBytes.length] = 0; // null terminator
            
            window.Module._web_ble_scan_res(idPtr, namePtr, 0);
            window.Module._free(idPtr);
            window.Module._free(namePtr);
        }

        await bleRxChar.startNotifications();
        bleRxChar.addEventListener('characteristicvaluechanged', (e) => {
            const value = new Uint8Array(e.target.value.buffer);
            if (window.Module && window.Module._web_ble_rx) {
                const len = value.length;
                const ptr = window.Module._malloc(len);
                window.Module.HEAPU8.set(value, ptr);
                window.Module._web_ble_rx(ptr, len, 0);
                window.Module._free(ptr);
            }
        });

        console.log("Web BLE connected and notifications started");
        if (window.Module && window.Module._web_ble_connected) {
            window.Module._web_ble_connected(0);
        }
        
    } catch (err) {
        console.error("Web BLE failed", err);
    }
};

window.webBleWrite = async (dataView) => {
    if (bleTxChar) {
        try {
            await bleTxChar.writeValueWithoutResponse(dataView);
        } catch(e) {
            console.error("Web BLE Write failed", e);
        }
    }
};

window.webBleDisconnect = () => {
    if (bleDevice && bleDevice.gatt.connected) {
        bleDevice.gatt.disconnect();
    }
};
