# CMake toolchain for 8-bit AVR targets built with avr-gcc.
#
# Chip selection (-mmcu=), CPU frequency (-DF_CPU=...) and any other
# board-specific flags are the board.cmake's responsibility, not this file's.
# This toolchain deliberately stays chip-agnostic so it can be reused across
# every AVR family (ATmega, ATtiny, ...).

set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

find_program(AVR_GCC     avr-gcc     REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_SIZE    avr-size    REQUIRED)

set(CMAKE_C_COMPILER   "${AVR_GCC}")
set(CMAKE_ASM_COMPILER "${AVR_GCC}")
set(CMAKE_OBJCOPY      "${AVR_OBJCOPY}" CACHE FILEPATH "avr-objcopy" FORCE)
set(CMAKE_SIZE         "${AVR_SIZE}"    CACHE FILEPATH "avr-size"    FORCE)

# CMake's compiler-detection step tries to LINK a tiny program by default,
# which fails on AVR without a chip and startup. Compile-only is enough to
# validate that the toolchain works.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Cross-compile: don't leak host paths into find_* calls.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
