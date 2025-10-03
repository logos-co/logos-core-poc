import os, strformat
import dynlib
import std/[strutils]

type
  AsyncCallbackC* = proc(result: cint, message: cstring, user_data: pointer) {.cdecl.}

  Param* = object
    name*: string
    value*: string
    ptype*: string

  LogosCallback* = proc(success: bool, message: string)

  CallbackHolder = ref object
    cb: LogosCallback

  PluginProxy* = object
    api*: LogosAPI
    name*: string

  LogosAPI* = ref object
    # dynamic library
    lib: LibHandle
    libPath: string
    pluginsDir: string
    isInitialized*: bool
    isStarted*: bool
    # function pointers
    p_logos_core_init: proc (argc: cint, argv: pointer) {.cdecl.}
    p_logos_core_set_plugins_dir: proc (dir: cstring) {.cdecl.}
    p_logos_core_start: proc () {.cdecl.}
    p_logos_core_exec: proc (): cint {.cdecl.}
    p_logos_core_cleanup: proc () {.cdecl.}
    p_logos_core_get_loaded_plugins: proc (): ptr cstring {.cdecl.}
    p_logos_core_get_known_plugins: proc (): ptr cstring {.cdecl.}
    p_logos_core_process_plugin: proc (pluginPath: cstring): cstring {.cdecl.}
    p_logos_core_load_plugin: proc (pluginName: cstring): cint {.cdecl.}
    p_logos_core_unload_plugin: proc (pluginName: cstring): cint {.cdecl.}
    p_logos_core_call_plugin_method_async: proc (pluginName, methodName, paramsJson: cstring, cb: AsyncCallbackC, userData: pointer) {.cdecl.}
    p_logos_core_register_event_listener: proc (pluginName, eventName: cstring, cb: AsyncCallbackC, userData: pointer) {.cdecl.}
    p_logos_core_process_events: proc () {.cdecl.}
    # callback holders to prevent GC
    pendingCallbacks: seq[CallbackHolder]
    eventCallbacks: seq[CallbackHolder]
    # event loop control (manual polling)
    intervalMs: int

var gCallback: AsyncCallbackC

proc requireSym[T](handle: LibHandle, name: string): T =
  let p = symAddr(handle, name)
  if p.isNil:
    quit &"Required symbol not found in liblogos_core: {name}", QuitFailure
  cast[T](p)

proc getLibExtension(): string =
  when defined(macosx): ".dylib"
  elif defined(windows): ".dll"
  else: ".so"

proc resolveDefaultPaths(): tuple[libPath: string, pluginsDir: string, logosHost: string] =
  let cwd = getCurrentDir()
  let root = cwd
  let coreBuild = root / "core" / "build"
  let libExt = getLibExtension()
  let libPath = coreBuild / "lib" / ("liblogos_core" & libExt)
  let pluginsDir = coreBuild / "modules"
  let exeExt = when defined(windows): ".exe" else: ""
  let logosHostBase = coreBuild / "bin" / ("logos_host" & exeExt)
  result = (absolutePath(libPath), absolutePath(pluginsDir), absolutePath(logosHostBase))

proc toJson*(params: openArray[Param]): string =
  var parts: seq[string] = @[]
  for p in params:
    parts.add("{\"name\":\"" & p.name & "\",\"value\":\"" & p.value & "\",\"type\":\"" & p.ptype & "\"}")
  result = "[" & parts.join(",") & "]"

proc inferParams*(values: openArray[string]): string =
  var params: seq[Param] = @[]
  for i, v in values:
    params.add(Param(name: "arg" & $i, value: v, ptype: "string"))
  result = toJson(params)

proc ffiCb(result: cint, message: cstring, userData: pointer) {.cdecl.} =
  let holder = cast[CallbackHolder](userData)
  let ok = (result == 1)
  let msg = if message.isNil: "" else: $message
  if holder != nil and holder.cb != nil:
    holder.cb(ok, msg)

proc loadLibrary(self: LogosAPI) =
  self.lib = loadLib(self.libPath)
  if self.lib.isNil:
    quit &"Failed to load library: {self.libPath}", QuitFailure

  self.p_logos_core_init = requireSym[proc (argc: cint, argv: pointer) {.cdecl.}](self.lib, "logos_core_init")
  self.p_logos_core_set_plugins_dir = requireSym[proc (dir: cstring) {.cdecl.}](self.lib, "logos_core_set_plugins_dir")
  self.p_logos_core_start = requireSym[proc () {.cdecl.}](self.lib, "logos_core_start")
  self.p_logos_core_exec = requireSym[proc (): cint {.cdecl.}](self.lib, "logos_core_exec")
  self.p_logos_core_cleanup = requireSym[proc () {.cdecl.}](self.lib, "logos_core_cleanup")
  self.p_logos_core_get_loaded_plugins = requireSym[proc (): ptr cstring {.cdecl.}](self.lib, "logos_core_get_loaded_plugins")
  self.p_logos_core_get_known_plugins = requireSym[proc (): ptr cstring {.cdecl.}](self.lib, "logos_core_get_known_plugins")
  self.p_logos_core_process_plugin = requireSym[proc (pluginPath: cstring): cstring {.cdecl.}](self.lib, "logos_core_process_plugin")
  self.p_logos_core_load_plugin = requireSym[proc (pluginName: cstring): cint {.cdecl.}](self.lib, "logos_core_load_plugin")
  self.p_logos_core_unload_plugin = requireSym[proc (pluginName: cstring): cint {.cdecl.}](self.lib, "logos_core_unload_plugin")
  self.p_logos_core_call_plugin_method_async = requireSym[proc (pluginName, methodName, paramsJson: cstring, cb: AsyncCallbackC, userData: pointer) {.cdecl.}](self.lib, "logos_core_call_plugin_method_async")
  self.p_logos_core_register_event_listener = requireSym[proc (pluginName, eventName: cstring, cb: AsyncCallbackC, userData: pointer) {.cdecl.}](self.lib, "logos_core_register_event_listener")
  self.p_logos_core_process_events = requireSym[proc () {.cdecl.}](self.lib, "logos_core_process_events")

proc convertCStringArray(ptrArr: ptr cstring): seq[string] =
  result = @[]
  if ptrArr.isNil: return
  let arr = cast[ptr UncheckedArray[cstring]](ptrArr)
  var i = 0
  while arr[i] != nil:
    result.add($arr[i])
    inc i

proc logosInit*(self: LogosAPI): bool

proc newLogosAPI*(libPath = "", pluginsDir = "", autoInit = true): LogosAPI =
  var paths = resolveDefaultPaths()
  result = LogosAPI(
    libPath: (if libPath.len > 0: libPath else: paths.libPath),
    pluginsDir: (if pluginsDir.len > 0: pluginsDir else: paths.pluginsDir),
    isInitialized: false,
    isStarted: false,
    pendingCallbacks: @[],
    eventCallbacks: @[],
    intervalMs: 100
  )
  putEnv("LOGOS_HOST_PATH", paths.logosHost)
  if autoInit:
    discard result.logosInit()

proc logosInit*(self: LogosAPI): bool =
  if self.isInitialized: return true
  if not fileExists(self.libPath):
    quit &"Library file not found at {self.libPath}", QuitFailure
  if not dirExists(self.pluginsDir):
    quit &"Plugins directory not found at {self.pluginsDir}", QuitFailure
  self.loadLibrary()
  if gCallback.isNil:
    gCallback = ffiCb
  self.p_logos_core_init(0, nil)
  self.p_logos_core_set_plugins_dir(self.pluginsDir.cstring)
  self.isInitialized = true
  result = true

proc start*(self: LogosAPI): bool =
  if not self.isInitialized: discard self.logosInit()
  if self.isStarted: return true
  self.p_logos_core_start()
  self.isStarted = true
  result = true

proc exec*(self: LogosAPI): int =
  if not self.isInitialized: discard self.logosInit()
  result = self.p_logos_core_exec()

proc cleanup*(self: LogosAPI) =
  if self.lib != nil and not self.p_logos_core_cleanup.isNil:
    self.p_logos_core_cleanup()
  if self.lib != nil:
    unloadLib(self.lib)
    self.lib = nil
  self.isInitialized = false
  self.isStarted = false

proc getLoadedPlugins*(self: LogosAPI): seq[string] =
  convertCStringArray(self.p_logos_core_get_loaded_plugins())

proc getKnownPlugins*(self: LogosAPI): seq[string] =
  convertCStringArray(self.p_logos_core_get_known_plugins())

proc getPluginStatus*(self: LogosAPI): tuple[loaded: seq[string], known: seq[string]] =
  (self.getLoadedPlugins(), self.getKnownPlugins())

proc processPlugin*(self: LogosAPI, pluginName: string): bool =
  let ext = getLibExtension()
  let candidate = self.pluginsDir / (pluginName & "_plugin" & ext)
  if not fileExists(candidate): return false
  let res = self.p_logos_core_process_plugin(candidate.cstring)
  result = (res != nil)

proc loadPlugin*(self: LogosAPI, pluginName: string): bool =
  self.p_logos_core_load_plugin(pluginName.cstring) == 1

proc unloadPlugin*(self: LogosAPI, pluginName: string): bool =
  self.p_logos_core_unload_plugin(pluginName.cstring) == 1

proc processAndLoadPlugins*(self: LogosAPI, pluginNames: openArray[string]): seq[tuple[name: string, processed: bool, loaded: bool]] =
  for name in pluginNames:
    let processed = self.processPlugin(name)
    var loaded = false
    if processed:
      loaded = self.loadPlugin(name)
    result.add((name: name, processed: processed, loaded: loaded))

proc callPluginMethodAsync*(self: LogosAPI, pluginName, methodName, paramsJson: string, cb: LogosCallback) =
  var holder = CallbackHolder(cb: cb)
  self.pendingCallbacks.add(holder)
  self.p_logos_core_call_plugin_method_async(pluginName.cstring, methodName.cstring, paramsJson.cstring, gCallback, cast[pointer](holder))

proc registerEventListener*(self: LogosAPI, pluginName, eventName: string, cb: LogosCallback) =
  var holder = CallbackHolder(cb: cb)
  self.eventCallbacks.add(holder)
  self.p_logos_core_register_event_listener(pluginName.cstring, eventName.cstring, gCallback, cast[pointer](holder))

proc processEventsTick*(self: LogosAPI) =
  if not self.isInitialized:
    discard self.logosInit()
  self.p_logos_core_process_events()

proc plugin*(self: LogosAPI, name: string): PluginProxy =
  result = PluginProxy(api: self, name: name)

proc call*(p: PluginProxy, methodName: string, paramsJsonOrValue: string, cb: LogosCallback) =
  # If the provided string already looks like a JSON array, assume it is the full
  # params JSON and pass through. Otherwise, treat it as a single string argument
  # and wrap it using the standard Param format.
  let s = paramsJsonOrValue.strip()
  if s.len > 0 and s[0] == '[':
    p.api.callPluginMethodAsync(p.name, methodName, s, cb)
  else:
    let wrapped = inferParams([paramsJsonOrValue])
    p.api.callPluginMethodAsync(p.name, methodName, wrapped, cb)

proc callStrings*(p: PluginProxy, methodName: string, values: openArray[string], cb: LogosCallback) =
  p.call(methodName, inferParams(values), cb)

proc on*(p: PluginProxy, eventName: string, cb: LogosCallback) =
  p.api.registerEventListener(p.name, eventName, cb)


