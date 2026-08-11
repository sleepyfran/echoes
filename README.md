# Echoes

Echoes is a music app that allows you to connect to multiple sources of music and play them directly through the app. It's album focused, meaning that instead of treating your library as a list of songs, it treats it as a list of albums, allowing you to shuffle albums, browse them based on genres, and more.

The project is split into a platform-agnostic `core` C++ library and per-platform
UIs. On Linux the UI is built with Qt 6 / KDE Kirigami (see `linux/`). On macOS
the UI is a native SwiftUI app (see `macos/`) that talks to `core` through an
Objective-C++ bridge.

Echoes is the (desktop) spiritual successor of [Echo](https://github.com/sleepyfran/echo).

## Before building

Make sure you create a `config.cmake` file on the root with the following format:
```
set(ONEDRIVE_AUTH_CLIENT_ID "your_client_id")
```

It is required by the build process to populate environment variables.

## Build on Linux

## Checks

Format the repository:

```bash
# Requires clang-format.
make format
```

Check formatting without changing files:

```bash
make format-check
```

Run C++ lint checks:

```bash
# Requires clang-tidy and the project build dependencies.
# Uses ~/kde/usr as the kde-builder install prefix by default.
make lint
```

If your KDE prefix is somewhere else:

```bash
make lint KDE_PREFIX=/path/to/kde/usr
```

### kde-builder

Compile and run the project with:

```bash
# Requires the project to be registered as `echoes` in your kde-builder config.
# Requires Qt 6, KDE Frameworks 6, Kirigami, and ECM dependencies to be available to kde-builder.
kde-builder echoes --run echoes-linux-ui

# Note: If you ever change `CMakeLists` and need to recreate the build directory use `--refresh-build`
```

### CMake

Compile:

```bash
# Requires Qt 6, KDE Frameworks 6, Kirigami, and Extra CMake Modules development packages.
cmake -S . -B build
cmake --build build
```

Run from the build directory:

```bash
./build/linux/echoes-linux-ui
```

Install and run from your shell:

```bash
cmake --install build
# Make sure the install prefix bin directory is in PATH.
echoes-linux-ui
```

## Build on macOS

The macOS app is an Xcode project that gets automatically created by CMake. In order to generate and open
the project in Xcode, execute the following commands:

```bash
cmake -G Xcode -S . -B build-macos
open build_macos/echoes.xcodeproj

# Note: If your (non-Xcode) IDE/LSP still depends on having a `build` folder with compile commands, also run:
cmake -S . -B build
```

Once it's opened, select `echoes_macos` as a target on the top and run the app.

## CLI

There's also a portable CLI for testing purposes. This one is not meant to be the actual product but a quick way of interacting
with the application core functionality:

```bash
cmake -S . -B build-cli -DECHOES_BUILD_LINUX_UI=OFF
cmake --build build-cli
./build-cli/cli/echoes-cli
```

## Current status and roadmap

Below is a list of things I'm currently working on or planning to work on:
- [ ] Basic file-based provider support (OneDrive only for now; file discovery, parsing, etc.)
- [ ] Basic remote playback support (OneDrive only for now; streaming, buffering, etc.)
- [ ] Bare-bones CLI for interacting with the core library (for testing purposes)
- [ ] Linux UI through KDE Kirigami
- [ ] macOS native UI through SwiftUI
- [ ] Spotify provider support
- [ ] Local files provider support
- [ ] Genre tagging and browsing
- [ ] Last.fm scrobbling support
- [ ] Album and artist shuffling
- [ ] Queue management
- [ ] Tag and metadata editing

## The hows and whys

### WHY C++ WHEN RUST IS THERE?

Simply because I wanted to finally learn C++ properly and wanted to feel in my own skin whether what people say about C++ being incredibly hard to learn and easy to misuse is true. Also, since cross-platform was the goal from the very beginning, using a language that is widely supported on all target platforms without too much fuzz was nice to have.

### AI usage

Since this is a learning project, I wanted to type each and every character of the code myself (except for some good ol' copy-pasta here and there). I did use AI for research, discovery of the std library and overall help in some topics, but I generally wrote everything myself. I know, right, on this day and age!

