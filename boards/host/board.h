#ifndef BOARD_H
#define BOARD_H

/* ========================================================================== */

/**
 * @brief Board binding for the host platform (native build, no MCU).
 *
 * Maps the generic PLATFORM_*_OPS macros to this board's concrete driver
 * ops instances, so application code can bind embedded-hal structs (via
 * GPIO_DEFINE, SERIAL_DEFINE, etc.) without knowing which target it's
 * compiled for. Only #include "board.h" — swapping boards/toolchains is
 * enough to pick a different set of ops for the same app source.
 *
 * @note  Backed by drivers/host, which has no real hardware behind it: ops
 *        print to stdout and track state in memory. Only meant to validate
 *        that application code builds and links natively.
 */

/* ========================================================================== */

#include "host.h"

/* ========================================================================== */

#define PLATFORM_GPIO_OPS host_gpio_ops

/* ========================================================================== */

#endif  // BOARD_H
