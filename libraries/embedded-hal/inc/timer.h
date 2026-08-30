#ifndef TIMER_H
#define TIMER_H

/* ========================================================================== */

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */

#include "../../inc/errno.h"

/* ========================================================================== */

/**
 * @brief Timer structure. This structure holds all information related to a
 * timer instance.
 */
struct timer;

/* ========================================================================== */

/**
 * @brief Prototype for the timer callback function.
 * @param context Pointer to user-defined context data (can be NULL).
 */
typedef void (*timer_callback_t)(void* context);

/* ========================================================================== */

/**
 * @brief Timer operations structure. This structure holds function pointers for
 * timer operations.
 */
struct timer_ops
{
    /**
     * @brief Blocking delay in milliseconds.
     * @param self Pointer to the timer structure.
     * @param ms Delay in milliseconds.
     * @return int8_t Returns 0 on success or -ERR on failure (see errno.h).
     */
    int8_t (*delay_ms)(const struct timer* self, uint16_t ms);

    /**
     * @brief Blocking delay in microseconds.
     * @param self Pointer to the timer structure.
     * @param us Delay in microseconds.
     * @return int8_t Returns 0 on success or -ERR on failure (see errno.h).
     */
    int8_t (*delay_us)(const struct timer* self, uint16_t us);

    /**
     * @brief Configures a timeout for the timer in milliseconds. When the
     * timeout expires, the callback function set by 'set_callback' will be
     * called.
     * @param self Pointer to the timer structure.
     * @param ms Timeout in milliseconds.
     * @return int8_t Returns 0 on success or -ERR on failure (see errno.h).
     */
    int8_t (*set_timeout_ms)(const struct timer* self, uint16_t ms);

    /**
     * @brief Resets the timer countdown.
     * @param self Pointer to the timer structure.
     */
    void (*reset_timeout)(const struct timer* self);

    /**
     * @brief Deactivates the timer.
     * @param self Pointer to the timer structure.
     */
    void (*deactivate_timeout)(const struct timer* self);

    /**
     * @brief Sets the callback function to be called when the timer expires.
     * @param self Pointer to the timer structure.
     * @param callback Pointer to the callback function.
     * @param context Pointer to user-defined context data (can be NULL).
     * @return int8_t Returns 0 on success or -ERR on failure (see errno.h).
     */
    int8_t (*set_timeout_callback)(
        const struct timer* self, timer_callback_t callback, void* context);
};

/* ========================================================================== */

struct timer
{
    /**
     * @brief Timer identifier. Serves to identify different timer peripherals
     * within the same driver.
     */
    uint8_t id;

    /**
     * @brief Pointer to the user-defined callback_context data (can be NULL).
     */
    void* context;

    /**
     * @brief Pointer to the timer operations structure. This structure must be
     * first created and initialized by the user. This structure must be of type
     * 'const struct timer_ops', which ensures that the function pointers cannot
     * be modified after initialization.
     */
    const struct timer_ops* const ops;
};

/* ========================================================================== */

/**
 * @brief Periodic tick source structure. Represents a hardware timer configured
 * to generate ticks at a fixed period.
 */
struct tick_source;

/* ========================================================================== */

/**
 * @brief Prototype for the tick source callback function. Called from the
 * timer's ISR context on every tick.
 * @param context Pointer to user-defined context data (can be NULL).
 */
typedef void (*tick_source_callback_t)(void* context);

/* ========================================================================== */

/**
 * @brief Tick source operations structure.
 */
struct tick_source_ops
{
    /**
     * @brief Configures the tick period and starts the tick source.
     * @param self Pointer to the tick source structure.
     * @param period_ms Period between ticks in milliseconds.
     * @return int8_t Returns 0 on success, -EINVAL if the requested period is
     * not achievable exactly by the underlying hardware, or -ERR on failure
     * (see errno.h).
     */
    int8_t (*initialize)(struct tick_source* self, uint32_t period_ms);

    /**
     * @brief Sets the callback function invoked from the ISR on every tick.
     * Pass NULL as callback to disable.
     * @param self Pointer to the tick source structure.
     * @param callback Callback function (or NULL to disable).
     * @param context Pointer to user-defined context data (can be NULL).
     * @return int8_t Returns 0 on success or -ERR on failure (see errno.h).
     */
    int8_t (*set_callback)(
        struct tick_source*    self,
        tick_source_callback_t callback,
        void*                  context);

    /**
     * @brief Consumes one pending tick. Pending ticks are accumulated in the
     * ISR, so back-to-back ticks are not lost between polls.
     * @param self Pointer to the tick source structure.
     * @return true if a tick was pending and consumed, false otherwise.
     */
    bool (*consume_tick)(struct tick_source* self);

    /**
     * @brief Returns the total number of ticks elapsed since initialize().
     * @note The counter is monotonic and wraps at 2^32 without notice
     * (~136 years at 1 s/tick, ~49 days at 1 ms/tick).
     * @param self Pointer to the tick source structure.
     * @return uint32_t Total ticks elapsed.
     */
    uint32_t (*get_ticks)(struct tick_source* self);
};

/* ========================================================================== */

struct tick_source
{
    /**
     * @brief Tick source identifier. Useful when several tick sources share the
     * same ops table.
     */
    uint8_t id;

    /**
     * @brief Pointer to the tick source operations structure.
     */
    const struct tick_source_ops* const ops;
};

/* ========================================================================== */

#endif
