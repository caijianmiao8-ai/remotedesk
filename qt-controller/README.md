# RemoteDesk Qt Controller

RemoteDesk Controller is a native Qt 6 desktop client that authenticates against the RemoteDesk backend, creates controller sessions, exchanges WebRTC signalling over Supabase Realtime, and streams host audio/video using [`libdatachannel`](https://github.com/paullouisageneau/libdatachannel).

## Features

- Device authorization login flow with `/api/device/start` and `/api/device/poll`
- Session lifecycle management (`/api/sessions/create`, `/api/sessions/join`, `/api/sessions/close`)
- Supabase Realtime (Phoenix) signalling for WebRTC offer/answer/ICE exchange
- WebRTC media playback via `libdatachannel`
- DataChannel for mouse/keyboard input events encoded as JSON

## Project Layout

```
qt-controller/
  CMakeLists.txt
  include/
    common/Protocol.h
    controller/
      App.h
      UiMainWindow.h
      AuthClient.h
      SignalingClient.h
      WebRtcPeer.h
  src/controller/
    App.cpp
    UiMainWindow.cpp
    AuthClient.cpp
    SignalingClient.cpp
    WebRtcPeer.cpp
  assets/
    icons/
      (placeholder for application icons)
```

## Build Requirements

- CMake ≥ 3.21
- C++17 compatible compiler
- [Qt 6](https://www.qt.io/) modules: Core, Gui, Widgets, Network, WebSockets, Multimedia
- [`libdatachannel`](https://libdatachannel.org/) and dependencies (`openssl`, `usrsctp`, `libsrtp`, `libyuv`, `opus`, `openh264`)
- [vcpkg](https://vcpkg.io/) (recommended) for dependency management

### Installing Dependencies with vcpkg (Windows x64)

```powershell
vcpkg install libdatachannel[openssl,libjuice,usrsctp] libyuv opus openh264 libsrtp openssl --triplet x64-windows
```

## Configure & Build

```powershell
# From the repository root
cmake -S qt-controller -B build -G "Ninja" \`
  -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \`
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The resulting executable is `build/Controller.exe` on Windows (or simply `Controller` on other platforms).

## Runtime Configuration

The default API base is baked into the binary:

```
https://ruoshui.fun.vercel.app
```

Future versions may support overriding this value through configuration files or command line arguments.

## Manual API Smoke Tests

Replace placeholders with actual values obtained during runtime.

```bash
# Start device authorization flow
curl -X POST https://ruoshui.fun.vercel.app/api/device/start \
  -H "content-type: application/json" \
  -d "{}"

# Poll for device approval
curl -X POST https://ruoshui.fun.vercel.app/api/device/poll \
  -H "content-type: application/json" \
  -d '{"device_code":"dc_xxx"}'

# Create a controller session
curl -X POST https://ruoshui.fun.vercel.app/api/sessions/create \
  -H "authorization: Bearer <app_token>" \
  -H "content-type: application/json" \
  -d '{"role":"controller"}'

# Fetch Supabase realtime credentials
curl "https://ruoshui.fun.vercel.app/api/realtime/signed-topic?sessionId=<id>" \
  -H "authorization: Bearer <app_token>"

# Fetch ICE servers
curl "https://ruoshui.fun.vercel.app/api/ice"
```

## Status

This repository contains the initial scaffolding for the RemoteDesk Controller. Networking, realtime signalling, WebRTC media handling, and UI flows are provided as a foundation and will continue to evolve toward the full M1 milestone.
