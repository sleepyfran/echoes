cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(echoes VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE AND (CMAKE_GENERATOR STREQUAL "Xcode" OR CMAKE_GENERATOR MATCHES "Ninja"))
    enable_language(Swift)
    add_subdirectory(macos)
endif()

add_subdirectory(core)

# The Kirigami/Qt UI is Linux-only. Default it ON only on Linux so that other
# platforms (e.g. macOS, where the UI lives in a native Xcode project) build
# just the core library without pulling in KDE dependencies.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(ECHOES_BUILD_LINUX_UI_DEFAULT ON)
else()
    set(ECHOES_BUILD_LINUX_UI_DEFAULT OFF)
endif()

option(ECHOES_BUILD_LINUX_UI "Build the Kirigami/Qt Linux UI" ${ECHOES_BUILD_LINUX_UI_DEFAULT})
if(ECHOES_BUILD_LINUX_UI)
    set(KDE_INSTALL_PREFIX_SCRIPT ON CACHE BOOL "Generate prefix.sh for kde-builder runtime setup")
    add_subdirectory(linux)
endif()
