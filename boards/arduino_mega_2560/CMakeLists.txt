# Header-only board: no board.c, just the header + the MCU driver dep.
# INTERFACE library propagates the include dir and the transitive link
# to atmegaxx0_1 (which brings in embedded-hal).

add_library(board INTERFACE)

target_include_directories(board INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(board INTERFACE atmegaxx0_1)
