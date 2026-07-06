# Echoes

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
