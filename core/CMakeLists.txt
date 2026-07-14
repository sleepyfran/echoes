file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")

add_library(echoes_core STATIC ${SOURCES} ${HEADERS})
add_library(echoes::core ALIAS echoes_core)
target_include_directories(echoes_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
    	${CMAKE_CURRENT_SOURCE_DIR}/third_party
)
target_link_libraries(echoes_core PRIVATE app_config)

find_package(OpenSSL REQUIRED)
target_compile_definitions(echoes_core PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(echoes_core
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
)

if(APPLE)
    target_link_libraries(echoes_core 
        PRIVATE 
        "-framework CoreFoundation" 
        "-framework Security"
    )
endif()

set_target_properties(echoes_core PROPERTIES
    XCODE_ATTRIBUTE_GCC_OPTIMIZATION_LEVEL[variant=Debug] "0"
)
