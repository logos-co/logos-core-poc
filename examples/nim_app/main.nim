import os, strformat
import ../../SDK/nim/logos_api

# Use the Nim SDK LogosAPI wrapper

proc main() =
  # Resolve core paths relative to this binary
  let appDir = getAppDir()
  let root = appDir / ".." / ".."  # examples/nim_app -> repo root
  let coreBuild = root / "core" / "build"
  let libExt = when defined(macosx): ".dylib" elif defined(windows): ".dll" else: ".so"
  let libPath = absolutePath(coreBuild / "lib" / ("liblogos_core" & libExt))
  let pluginsDir = absolutePath(coreBuild / "modules")

  var api = newLogosAPI(libPath = libPath, pluginsDir = pluginsDir, autoInit = true)
  discard api.start()

  # Load plugins
  let results = api.processAndLoadPlugins(["capability_module", "waku_module", "chat"])
  for r in results:
    echo &"plugin={r.name} processed={r.processed} loaded={r.loaded}"

  # Pump events manually for a bit
  for i in 0..10:
    api.processEventsTick()
    sleep 50

  # Register events
  api.plugin("chat").on("chatMessage") do (success: bool, message: string):
    echo &"[chatMessage] success={success} message={message}"

  api.plugin("chat").on("historyMessage") do (success: bool, message: string):
    echo &"[historyMessage] success={success} message={message}"

  # Initialize chat
  api.plugin("chat").call("initialize", "[]") do (success: bool, message: string):
    echo &"[initialize] success={success} message={message}"

  for i in 0..20:
    api.processEventsTick()
    sleep 50

  # Join a default channel and request history
  let channel = "baixa-chiado"
  let oneParam = "[{\"name\":\"arg0\",\"value\":\"" & channel & "\",\"type\":\"string\"}]"
  api.plugin("chat").call("joinChannel", oneParam) do (success: bool, message: string):
    echo &"[joinChannel] success={success} message={message}"

  for i in 0..10:
    api.processEventsTick()
    sleep 50
  api.plugin("chat").call("retrieveHistory", oneParam) do (success: bool, message: string):
    echo &"[retrieveHistory] success={success} message={message}"

  # Keep pumping events so callbacks arrive
  while true:
    api.processEventsTick()
    sleep 50

when isMainModule:
  try:
    main()
  finally:
    discard