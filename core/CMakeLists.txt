file(GLOB_RECURSE SOURCES "src/*.cpp")

add_library(echoes_core STATIC ${SOURCES})

add_library(echoes::core ALIAS echoes_core)

target_include_directories(echoes_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    	${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
