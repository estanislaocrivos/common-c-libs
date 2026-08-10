# Host driver: stdout-backed stand-ins for the embedded-hal ops structs, for
# compiling and linking application code natively (no cross toolchain, no
# real hardware behind it). Not a substitute for the Ceedling/CMock unit
# tests under each library's test/ directory.

if(NOT TARGET embedded-hal)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../libraries/embedded-hal
        ${CMAKE_BINARY_DIR}/embedded-hal)
endif()

file(GLOB HOST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(host STATIC ${HOST_SOURCES})

target_include_directories(host PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(host PUBLIC embedded-hal)

target_compile_features(host PRIVATE c_std_99)
target_compile_options(host PRIVATE -Wall -Wextra)
