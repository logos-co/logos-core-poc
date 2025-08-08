import os, strformat, strutils
import dynlib

# Minimal Nim client that loads liblogos_core and interacts with plugins

type
  AsyncCallback* = proc(result: cint, message: cstring, user_data: pointer) {.cdecl.}

var
  lib: LibHandle
  logos_core_init: proc(argc: cint, argv: pointer) {.cdecl.}
  logos_core_set_plugins_dir: proc(dir: cstring) {.cdecl.}
  logos_core_start: proc() {.cdecl.}
  logos_core_exec: proc(): cint {.cdecl.}
  logos_core_cleanup: proc() {.cdecl.}
  logos_core_process_plugin: proc(pluginPath: cstring): cstring {.cdecl.}
  logos_core_load_plugin: proc(pluginName: cstring): cint {.cdecl.}
  logos_core_unload_plugin: proc(pluginName: cstring): cint {.cdecl.}
  logos_core_call_plugin_method_async: proc(pluginName, methodName, paramsJson: cstring, cb: AsyncCallback, userData: pointer) {.cdecl.}
  logos_core_register_event_listener: proc(pluginName, eventName: cstring, cb: AsyncCallback, userData: pointer) {.cdecl.}
  logos_core_process_events: proc() {.cdecl.}

proc requireSym[T](handle: LibHandle, name: string): T =
  let p = symAddr(handle, name)
  if p.isNil:
    quit &"Required symbol not found in liblogos_core: {name}", QuitFailure
  cast[T](p)

proc getLibExtension(): string =
  when defined(macosx): ".dylib"
  elif defined(windows): ".dll"
  else: ".so"

proc getPluginExtension(): string =
  # Mirrors getLibExtension for plugins
  getLibExtension()

proc resolvePaths(): tuple[libPath: string, pluginsDir: string, logosHost: string] =
  let appDir = getAppDir()
  let root = appDir / ".." / ".."  # examples/nim_app -> project root
  let coreBuild = root / "core" / "build"
  let libExt = getLibExtension()
  let libPath = coreBuild / "lib" / ("liblogos_core" & libExt)
  let pluginsDir = coreBuild / "modules"
  let exeExt = when defined(windows): ".exe" else: ""
  let logosHostBase = coreBuild / "bin" / ("logos_host" & exeExt)
  result = (absolutePath(libPath), absolutePath(pluginsDir), absolutePath(logosHostBase))

proc printStatus(prefix: string) =
  echo prefix

let
  tagInitializeStr = "initialize"
  tagJoinStr = "joinChannel"
  tagHistoryStr = "retrieveHistory"
  tagEvtChatMsgStr = "chatMessage"
  tagEvtHistoryMsgStr = "historyMessage"
  tagInitializePtr = cstring(tagInitializeStr)
  tagJoinPtr = cstring(tagJoinStr)
  tagHistoryPtr = cstring(tagHistoryStr)
  tagEvtChatMsgPtr = cstring(tagEvtChatMsgStr)
  tagEvtHistoryMsgPtr = cstring(tagEvtHistoryMsgStr)

proc asyncCb(res: cint, msg: cstring, userData: pointer) {.cdecl.} =
  let ok = (res == 1)
  let tag = if userData.isNil: "cb" else: $cast[cstring](userData)
  echo &"[{tag}] success={ok} message={msg}"

proc ensurePaths(libPath, pluginsDir, logosHost: string) =
  if not fileExists(libPath):
    quit &"Library not found at {libPath}. Build the core first (./scripts/run_core.sh build).", QuitFailure
  if not dirExists(pluginsDir):
    quit &"Plugins directory not found at {pluginsDir}. Build the modules first.", QuitFailure
  putEnv("LOGOS_HOST_PATH", logosHost)

proc loadCore(libPath: string) =
  lib = loadLib(libPath)
  if lib.isNil:
    quit &"Failed to load library: {libPath}", QuitFailure

  logos_core_init = requireSym[proc (argc: cint, argv: pointer) {.cdecl.}](lib, "logos_core_init")
  logos_core_set_plugins_dir = requireSym[proc (dir: cstring) {.cdecl.}](lib, "logos_core_set_plugins_dir")
  logos_core_start = requireSym[proc () {.cdecl.}](lib, "logos_core_start")
  logos_core_exec = requireSym[proc (): cint {.cdecl.}](lib, "logos_core_exec")
  logos_core_cleanup = requireSym[proc () {.cdecl.}](lib, "logos_core_cleanup")
  logos_core_process_plugin = requireSym[proc (pluginPath: cstring): cstring {.cdecl.}](lib, "logos_core_process_plugin")
  logos_core_load_plugin = requireSym[proc (pluginName: cstring): cint {.cdecl.}](lib, "logos_core_load_plugin")
  logos_core_unload_plugin = requireSym[proc (pluginName: cstring): cint {.cdecl.}](lib, "logos_core_unload_plugin")
  logos_core_call_plugin_method_async = requireSym[proc (pluginName, methodName, paramsJson: cstring, cb: AsyncCallback, userData: pointer) {.cdecl.}](lib, "logos_core_call_plugin_method_async")
  logos_core_register_event_listener = requireSym[proc (pluginName, eventName: cstring, cb: AsyncCallback, userData: pointer) {.cdecl.}](lib, "logos_core_register_event_listener")
  logos_core_process_events = requireSym[proc () {.cdecl.}](lib, "logos_core_process_events")

proc processAndLoadPlugins(pluginsDir: string, plugins: openArray[string]) =
  let ext = getPluginExtension()
  for name in plugins:
    let candidate = pluginsDir / (name & "_plugin" & ext)
    if not fileExists(candidate):
      echo &"[plugins] Missing file for {name}: {candidate}"
      continue
    let processed = logos_core_process_plugin(candidate.cstring)
    if processed.isNil:
      echo &"[plugins] Failed to process: {name}"
      continue
    let loaded = logos_core_load_plugin(name.cstring)
    if loaded == 1:
      echo &"[plugins] Loaded: {name}"
    else:
      echo &"[plugins] Failed to load: {name}"

proc startEventPump(intervalMs = 50) =
  while true:
    logos_core_process_events()
    sleep intervalMs

proc main() =
  let (libPath, pluginsDir, logosHost) = resolvePaths()
  ensurePaths(libPath, pluginsDir, logosHost)

  printStatus &"Using lib: {libPath}\nUsing plugins dir: {pluginsDir}\nUsing logos_host: {logosHost}"

  loadCore(libPath)

  # Initialize and start core
  logos_core_init(0, nil)
  logos_core_set_plugins_dir(pluginsDir)
  logos_core_start()

  # Process and load capability_module, waku_module, chat
  processAndLoadPlugins(pluginsDir, ["capability_module", "waku_module", "chat"]) 

  # Register chat events
  logos_core_register_event_listener("chat", "chatMessage", asyncCb, cast[pointer](tagEvtChatMsgPtr))
  logos_core_register_event_listener("chat", "historyMessage", asyncCb, cast[pointer](tagEvtHistoryMsgPtr))

  # Initialize chat
  logos_core_call_plugin_method_async("chat", "initialize", "[]", asyncCb, cast[pointer](tagInitializePtr))

  # Give time for initialization
  sleep 1000

  # Join a default channel and request history
  let channel = "baixa-chiado"
  # Craft params JSON explicitly: [{"name":"arg0","value":"<channel>","type":"string"}]
  let joinParams = "[{\"name\":\"arg0\",\"value\":\"" & channel & "\",\"type\":\"string\"}]"
  let historyParams = joinParams
  logos_core_call_plugin_method_async("chat", "joinChannel", joinParams.cstring, asyncCb, cast[pointer](tagJoinPtr))
  sleep 500
  logos_core_call_plugin_method_async("chat", "retrieveHistory", historyParams.cstring, asyncCb, cast[pointer](tagHistoryPtr))

  # Start non-blocking event pump
  startEventPump(50)

when isMainModule:
  try:
    main()
  finally:
    if not lib.isNil:
      # Best-effort cleanup
      if not logos_core_cleanup.isNil:
        logos_core_cleanup()
      unloadLib(lib)