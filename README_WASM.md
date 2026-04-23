# VESC Tool WebAssembly Build Notes

## Qt 6.8 CMake Architecture
As of our latest major architectural upgrade, the WebAssembly port of VESC Tool has been fully migrated away from legacy Qt 5 / QMake configurations. 

We now leverage the modern **Qt 6.8.1 WebAssembly environment** alongside standard `CMake` declarations. WebAssembly flags, including `ASYNCIFY` integrations and `idbfs_pre.js` hooks, are exclusively bundled into `CMakeLists.txt` via `EMSCRIPTEN` platform conditionals.

When building the Wasm pipeline via Docker or compiling it locally in your own environment, you **must use Emscripten 3.1.70** (which strictly mirrors Qt 6.8.1 parity).

## Persistence and Settings (Native IDBFS)
Historically (under Qt 5.15), VESC Tool was forced to utilize a manual polling `localStorage` bridge injected directly inside `main.cpp` because Qt 5's internal `QSettings` destructor triggered fatal unwinding collisions when mixed with Emscripten's `ASYNCIFY` yielding and asynchronous `IDBFS` transactions.

With the migration to Qt 6.8:
1. Native `QSettings` hooks are heavily bulletproofed and operate effectively with native JS HTML5 bindings under the hood without triggering stack unwind traps!
2. The manual `EM_ASM` localStorage bridge has been completely deleted.
3. We actively hook `idbfs_pre.js` via the `CMakeLists.txt` linker flag (`--pre-js`) to construct the user's `MEMFS` instance (mounted at `/home/web_user`). This correctly maps and persists traditional C++ `QFile` operations (like saving Logs and Motor Profiles out of the virtual filesystem) asynchronously into the browser's IndexedDB.

### Memory Allocation
Because QML and comprehensive desktop layouts can still prove memory-hungry during high-load render bootstrapping, we explicitly configure Qt 6 to allocate `256MB` of initial memory footprint right out of the gate by setting `QT_WASM_INITIAL_MEMORY` inside target properties!
