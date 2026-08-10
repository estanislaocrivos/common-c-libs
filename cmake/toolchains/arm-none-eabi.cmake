# CMake toolchain for ARM Cortex-M targets built with arm-none-eabi-gcc.
#
# CPU/FPU flags (-mcpu=, -mfpu=, -mfloat-abi=), linker script and startup
# source are the board.cmake's responsibility. This file only wires up the
# toolchain binaries.

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(ARM_GCC     arm-none-eabi-gcc     REQUIRED)
find_program(ARM_GXX     arm-none-eabi-g++     REQUIRED)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE    arm-none-eabi-size    REQUIRED)

set(CMAKE_C_COMPILER   "${ARM_GCC}")
set(CMAKE_CXX_COMPILER "${ARM_GXX}")
set(CMAKE_ASM_COMPILER "${ARM_GCC}")
set(CMAKE_OBJCOPY      "${ARM_OBJCOPY}" CACHE FILEPATH "arm objcopy" FORCE)
set(CMAKE_SIZE         "${ARM_SIZE}"    CACHE FILEPATH "arm size"    FORCE)

# See avr.cmake — the compiler-detection test-link needs a startup+ldscript
# we don't have at this stage. Compile-only is enough.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Newlib-nano is the standard "no host" libc for bare-metal Cortex-M. Enable
# it by default; boards that need full newlib can override.
set(CMAKE_C_FLAGS_INIT   "--specs=nano.specs")
set(CMAKE_CXX_FLAGS_INIT "--specs=nano.specs")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
