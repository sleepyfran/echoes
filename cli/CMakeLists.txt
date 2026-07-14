file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(echoes-cli ${SOURCES})
target_include_directories(echoes-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(echoes-cli PRIVATE echoes::core)
