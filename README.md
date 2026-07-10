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

