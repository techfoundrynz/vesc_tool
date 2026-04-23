# VESC Tool WebAssembly Build Notes

## Persistence and Settings (IDBFS vs localStorage)

By default, Qt 5 WebAssembly applications typically use Emscripten's `IDBFS` (IndexedDB File System) to persist `QSettings` across page reloads.

However, the VESC Tool requires Emscripten's `-s ASYNCIFY=1` flag in order to suspend the C++ event loop and wait for asynchronous Web Serial / Web Bluetooth APIs without blocking the browser thread.

**Extremely Important Bug:**
There is a known, fatal bug in Emscripten 1.39 where compiling with **both** `ASYNCIFY=1` and `lidbfs.js` enabled causes stack unwinding conflicts. When the `QSettings` destructor internally fires `FS.syncfs(false)` inside an asynchronous IndexedDB callback (`transaction.oncomplete`), the Stack Unwinding logic misinterprets the C++ boundary execution context, leading to a fatal `abort(RuntimeError: unreachable)` trap. 

### Resolution: The LocalStorage Shim

To build the project cleanly without arbitrary runtime lockups:
1. `IDBFS` has been aggressively disabled and removed from the `vesc_tool.pro` WebAssembly linker flags.
2. During initialization (in `main.cpp`), before Qt tries to instantiate any `QSettings` object, we utilize `EM_ASM` to inject a JavaScript bridge that parses `window.localStorage.getItem('vesc_settings_conf')`.
3. If settings exist in the browser, C++ manually constructs the Qt `.config` directory structure inside Emscripten's volatile `MEMFS` and writes the localStorage string payload as an `utf8` initialized configuration file.
4. Periodically (every 5 seconds via `setInterval`), we execute a JavaScript polling loop that watches Qt's internal `MEMFS` system and flushes any internal `QSettings` configuration mutation directly into `window.localStorage` under the `vesc_settings_conf` key.

This ensures seamless `QSettings` parity with Native mode, whilst completely sidestepping Emscripten's asynchronous transaction traps! 

### Memory Growth Limit Fix
By default, the project was previously setting a hard limit for Memory on Boot via `QMAKE_LFLAGS += -s TOTAL_MEMORY=134217728` (128mb limits). The desktop layout instantiation combined with QML engines routinely exhausts this allocation limit during Heavy Boot loads resulting in silent `memory access out of bounds` Segmentation Faults. 

To permanently banish out of memory issues, `-s ALLOW_MEMORY_GROWTH=1` has been formally set inside `vesc_tool.pro`- replacing manual memory constraints!
