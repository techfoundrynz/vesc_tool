# VESC Tool WebAssembly (Browser) Build Guide

This guide details compiling the VESC Tool into a browser-native WebAssembly (WASM) payload.

## Core Requirements

Because of how deeply the low-level Javascript C-Bindings hook into Qt's WebBridge, **you must use the exact matching versions** of the toolchains:

* **Qt Framework**: `5.15.2` (Target Architecture: `wasm_32`)
* **Emscripten (emsdk)**: `1.39.8`

---

## 🐋 Building via Docker (Recommended)

The simplest and safest way to build the project without polluting your host installation or fighting version mismatches is by using Docker.

### 1. Build the Container Image
From the root of the repository, build the image locally. This only needs to be done once:
```bash
docker build -t vesc_tool_wasm -f Dockerfile.wasm .
```

### 2. Run the Compiler 
Run the container and mount your repository into it. The container will automatically load the local repository state, run `qmake` and `make j8` on it, and output the compiled binaries back to your `web/dist/` folder.
```bash
docker run --rm -v $(pwd):/workspace vesc_tool_wasm
```

---

## 🐧 Building Locally (Linux / WSL2)

If you intend to work on the UI iteratively, building natively on WSL/Linux is slightly faster.

### 1. Download & Activate Emscripten `1.39.8`
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install 1.39.8
./emsdk activate 1.39.8
source ./emsdk_env.sh
```

### 2. Download Qt 5.15.2 WASM Edition
Using Python's unofficial headless Qt installer `aqtinstall`:
```bash
pip3 install aqtinstall
python3 -m aqt install-qt linux desktop 5.15.2 wasm_32
```
*Note: You must ensure you run `qmake` from the `/wasm_32/bin/qmake` folder during setup!*

### 3. Compile the Application
Once the `emsdk_env.sh` is sourced and `qmake` is in your `$PATH` / executable context, simply run the automated build script:
```bash
./build_web.sh
```

---

## 🚀 Running / Testing the Output

Once built via Docker or Locally, the payload (e.g. `vesc_tool_7.00.js`) is dumped statically into the `./web/dist/` folder. 

1. Launch the local testing server:
```bash
./start_web.sh
```

2. Open Chromium (Chrome, Edge, Opera, Brave) and navigate to **[http://localhost:8000](http://localhost:8000)**.
*(Note: It must exactly be `localhost`—and NOT `127.0.0.1`! Chromium-based browsers mandate Secure Contexts (HTTPS) to unlock the Web Serial and Web Bluetooth APIs, but `localhost` acts as a recognized developer bypass).*
