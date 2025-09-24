---
title: Package Manager Module
description: Details about dependency management through the package manager module.
---

# Package Manager Module

## Overview

The Package Manager Module installs and enumerates third‑party plug‑ins. It copies plug‑in files into the core plug‑ins directory and requests the core to process their metadata so they become available to the system. Currently it simply scans a packages folder to list available plug‑ins and their metadata, this module can be updated in the future into somethign more complex while keeping some of the same APIs.

## API

### Methods

- `installPlugin(pluginPath: string)` → `bool`
  - Copies the plug‑in at `pluginPath` into the configured plug‑ins directory and asks the core to process it (via `core_manager.processPlugin`).
  - Returns `true` if installation and processing succeed.

- `getPackages()` → `QJsonArray`
  - Scans the packages directory for `.so/.dll/.dylib` files, reads each plug‑in's metadata, and returns an array of JSON objects describing them.
  - Convenience method; not part of the minimal interface but available in the module.

- `initLogos(logosAPI)`
  - Stores the `LogosAPI` pointer so the module can call other modules (e.g., `core_manager`).
  - Called automatically during plug‑in initialization.

### Events

- No custom events are emitted by this module at present.

### Notes

- `installPlugin` expects a path to a valid plug‑in file; any extra files (in the same directory) declared in its metadata are copied alongside it.
- The current implementation assumes packages reside in a specific directory; this can be swapped for a network‑backed source in the future.

## Examples

### C++

Using generated `C++` wrappers from another module

```cpp
// Initialize and install a plug‑in from a local path
const bool ok = logos.package_manager.installPlugin("/path/to/plugin.dylib");
qInfo() << "installPlugin:" << ok;

// List available packages (JSON array)
const auto pkgs = logos.package_manager.getPackages();
qInfo() << "packages count:" << pkgs.size();
```

### Javascript

Using the `JavaScript` SDK (reflective API)

```javascript
const LogosAPI = require('logos-api');

const logos = new LogosAPI();
logos.start();
logos.processAndLoadPlugins(["package_manager"]);
logos.startEventProcessing();

(async () => {
  // Install a plugin
  const ok = await logos.package_manager.installPlugin("/path/to/plugin.dylib");
  console.log("installPlugin:", ok);

  // Enumerate available packages
  const pkgs = await logos.package_manager.getPackages();
  console.log("packages:", pkgs);
})();
```
