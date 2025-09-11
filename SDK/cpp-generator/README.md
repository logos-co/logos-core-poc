logos-cpp-generator
===================

A small CLI that loads a Logos module Qt plug-in (.so/.dylib/.dll) and prints its invokable methods as JSON.

Build
-----

Build this target standalone:

```bash
cmake -S SDK/cpp-generator -B build/cpp-generator
cmake --build build/cpp-generator --target logos-cpp-generator
```

The binary will be at build/cpp-generator/bin/logos-cpp-generator.

Usage
-----

```bash
build/cpp-generator/bin/logos-cpp-generator /absolute/path/to/your_module_plugin.ext
```

It outputs a JSON array of method descriptors including signature, name, returnType, isInvokable and parameters (with type and name).

Notes
-----
- Pass the full path to the plug-in file. The tool will try the provided path as-is.
- The tool uses QPluginLoader and introspects the QObject metaobject to enumerate methods.


