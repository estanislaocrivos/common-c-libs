# MCU-family driver library for ATmega640/1280/1281/2560/2561.
#
# Requires -mmcu=<chip> and -DF_CPU=<hz> to be set by the app (via
# add_compile_options in the top-level CMakeLists after including board.cmake).
# Without those, avr/io.h can't select the correct register map and this
# library won't build.

# Self-sufficient like the board layer: pulls in embedded-hal itself if
# whatever included this driver hasn't already added it.
if(NOT TARGET embedded-hal)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../libraries/embedded-hal
        ${CMAKE_BINARY_DIR}/embedded-hal)
endif()

file(GLOB ATMEGAXX0_1_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(atmegaxx0_1 STATIC ${ATMEGAXX0_1_SOURCES})

target_include_directories(atmegaxx0_1 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(atmegaxx0_1 PUBLIC embedded-hal)

target_compile_features(atmegaxx0_1 PRIVATE c_std_99)
target_compile_options(atmegaxx0_1 PRIVATE -Wall -Wextra)
