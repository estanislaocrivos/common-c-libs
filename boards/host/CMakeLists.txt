# Header-only board: no board.c, just the header + the host driver dep.
# INTERFACE library propagates the include dir and the transitive link
# to host (which brings in embedded-hal).
#
# The guard makes this board self-sufficient: it works whether it's reached
# through the repo's root CMakeLists.txt (-DBOARD=host) or by an app
# add_subdirectory-ing this folder directly.
if(NOT TARGET host)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../drivers/host
        ${CMAKE_BINARY_DIR}/host)
endif()

add_library(board INTERFACE)

target_include_directories(board INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(board INTERFACE host)
