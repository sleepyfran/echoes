add_library(echoes_core STATIC
    src/greeting.cpp
)

add_library(echoes::core ALIAS echoes_core)

target_include_directories(echoes_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
