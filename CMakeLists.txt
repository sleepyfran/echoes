cmake_minimum_required(VERSION 3.16)

project(echoes VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(KDE_INSTALL_PREFIX_SCRIPT ON CACHE BOOL "Generate prefix.sh for kde-builder runtime setup")

add_subdirectory(core)

option(ECHOES_BUILD_LINUX_UI "Build the Kirigami/Qt Linux UI" ON)
if(ECHOES_BUILD_LINUX_UI)
    add_subdirectory(linux)
endif()
