# MCU-family driver library for STM32F4xx.
#
# Requires CMSIS-Device headers (stm32f4xx.h and friends) on the include
# path — this driver does not vendor CMSIS. The board.cmake is expected
# either to provide CMSIS via a package (STM32CubeF4 etc.) or to add the
# include directory to this target after add_subdirectory.
#
# Requires CPU/FPU flags (-mcpu, -mfpu, -mfloat-abi) to be set by the app,
# same pattern as the AVR side.

# Self-sufficient like the board layer: pulls in embedded-hal itself if
# whatever included this driver hasn't already added it.
if(NOT TARGET embedded-hal)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../libraries/embedded-hal
        ${CMAKE_BINARY_DIR}/embedded-hal)
endif()

file(GLOB STM32F4XX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(stm32f4xx STATIC ${STM32F4XX_SOURCES})

target_include_directories(stm32f4xx PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(stm32f4xx PUBLIC embedded-hal)

target_compile_features(stm32f4xx PRIVATE c_std_99)
target_compile_options(stm32f4xx PRIVATE -Wall -Wextra)
