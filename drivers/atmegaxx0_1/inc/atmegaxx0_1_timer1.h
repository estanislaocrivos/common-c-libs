#ifndef ATMEGAXX0_1_TIMER1_H
#define ATMEGAXX0_1_TIMER1_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief tick_source_ops implementation on top of the 16-bit Timer/Counter1
 *        peripheral of the ATmega640/1280/1281/2560/2561 family. Bind a
 *        struct tick_source to &atmegaxx0_1_timer1_tick_source_ops to use it.
 *
 * @note  Uses CTC mode 4 (WGM13:0 = 0100) with OCR1A as TOP, so the counter
 *        auto-clears on every match and no drift accumulates. The compare
 *        match A interrupt (TIMER1_COMPA_vect) drives the tick counter and
 *        the optional user callback.
 *
 * @note  `initialize(period_ms)` picks the smallest prescaler in {1, 8, 64,
 *        256, 1024} for which OCR1A can represent the requested period
 *        exactly (highest counter resolution, still exact). If no combination
 *        is exact -- e.g. an F_CPU not commensurable with the requested
 *        period, or a period longer than (65536 * 1024) / F_CPU -- it returns
 *        -EINVAL, matching the interface contract. `period_ms == 0` is also
 *        -EINVAL.
 *
 * @note  There is one physical TMR1 peripheral, so this driver keeps a
 *        single set of internal state (tick counters, callback). Binding
 *        more than one struct tick_source to these ops is not supported;
 *        the most recent initialize() wins.
 *
 * @note  This driver enables OCIE1A but does not enable the global interrupt
 *        flag. The application must call sei() itself.
 */
extern const struct tick_source_ops atmegaxx0_1_timer1_tick_source_ops;

/* ========================================================================== */

#endif  // ATMEGAXX0_1_TIMER1_H
