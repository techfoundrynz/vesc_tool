Module.preRun.push(function() {
    try { FS.mkdir('/home/web_user'); } catch(e) {}
    FS.mount(IDBFS, {}, '/home/web_user');
    
    addRunDependency('syncfs');
    FS.syncfs(true, function(err) {
        if (err) {
            console.error("IDBFS mount error:", err);
            if (err.name === 'VersionError' || err.message.includes('VersionError')) {
                console.log("Auto-resolving IndexDB version mismatch...");
                var idb_names = ['emscripten_fs', '/home/web_user', '/home/web_user/.config/VESC'];
                idb_names.forEach(name => indexedDB.deleteDatabase(name));
                
                if (!sessionStorage.getItem('idbfs_auto_restarted')) {
                    sessionStorage.setItem('idbfs_auto_restarted', '1');
                    window.location.reload();
                    return;
                } else if (typeof showStartupError === 'function') {
                    showStartupError("Settings database corrupted. Please open Developer Tools -> Application -> Clear Site Data.");
                }
            }
        }
        removeRunDependency('syncfs');
    });
});
