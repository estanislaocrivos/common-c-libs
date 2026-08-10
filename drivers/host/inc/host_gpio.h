#ifndef HOST_GPIO_H
#define HOST_GPIO_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief gpio_ops implementation for the host platform. There is no real
 *        hardware behind this: every op prints what it would have done to
 *        stdout and tracks pin state in memory. Pass &host_gpio_ops to
 *        GPIO_DEFINE as the ops_ptr argument.
 *
 * @note  Only meant to validate that application code written against the
 *        generic embedded-hal GPIO API compiles and links on the host
 *        (e.g. for compile-time checks or running app-level logic under a
 *        native/CI build). Not a substitute for the unit-test mocks under
 *        each library's test/ directory.
 */
extern const struct gpio_ops host_gpio_ops;

/* ========================================================================== */

/**
 * @brief Number of virtual pin slots tracked by this driver. Pin IDs are
 *        arbitrary on the host (no real port/pin layout to map to) — pick
 *        any value in [0, HOST_GPIO_PIN_COUNT) in your application.
 */
#define HOST_GPIO_PIN_COUNT 32

/* ========================================================================== */

#endif  // HOST_GPIO_H
