# CONFIGURE_DEPENDS makes CMake re-glob and regenerate the Xcode project
# automatically on every build, so files added/removed/renamed on disk (e.g.
# from within Xcode) are picked up without manually rerunning `cmake -G Xcode`.
file(GLOB_RECURSE SWIFT_SOURCES CONFIGURE_DEPENDS "src/*.swift")
file(GLOB_RECURSE BRIDGE_SOURCES CONFIGURE_DEPENDS "CoreBridge/*.mm" "CoreBridge/*.h")

add_executable(echoes-macos MACOSX_BUNDLE 
    ${SWIFT_SOURCES} 
    ${BRIDGE_SOURCES}
)

target_link_libraries(echoes-macos PRIVATE echoes::core)

set_target_properties(echoes-macos PROPERTIES
    XCODE_ATTRIBUTE_SWIFT_VERSION "6.2.1"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++20"
    XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/CoreBridge/CoreBridge-Bridging-Header.h"
    XCODE_ATTRIBUTE_SWIFT_OPTIMIZATION_LEVEL[variant=Debug] "-Onone"
    XCODE_ATTRIBUTE_SWIFT_COMPILATION_MODE[variant=Debug] "singlefile"
    XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Debug] "0"
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
    MACOSX_BUNDLE_GUI_IDENTIFIER "me.sleepyfran.echoes"
    MACOSX_BUNDLE_BUNDLE_NAME "Echoes"
)

# target_sources(echoes-macos PRIVATE 
#     "Resources/Assets.xcassets" 
#     "Resources/Info.plist"
# )
#
# set_target_properties(echoes-macos PROPERTIES
#     MACOSX_BUNDLE_INFO_PLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
# )
