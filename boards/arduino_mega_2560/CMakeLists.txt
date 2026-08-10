# Header-only board: no board.c, just the header + the MCU driver dep.
# INTERFACE library propagates the include dir and the transitive link
# to atmegaxx0_1 (which brings in embedded-hal).
#
# The guard makes this board self-sufficient: it works whether it's reached
# through the repo's root CMakeLists.txt (-DBOARD=arduino_mega_2560) or by
# an app add_subdirectory-ing this folder directly.
if(NOT TARGET atmegaxx0_1)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../drivers/atmegaxx0_1
        ${CMAKE_BINARY_DIR}/atmegaxx0_1)
endif()

add_library(board INTERFACE)

target_include_directories(board INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(board INTERFACE atmegaxx0_1)
