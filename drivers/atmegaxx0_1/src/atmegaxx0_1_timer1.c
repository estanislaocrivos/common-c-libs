#include "../inc/atmegaxx0_1_timer1.h"

#include <util/atomic.h>

/* ========================================================================== */

/* Prescaler options for TMR1: {divider, CS12:0 bit pattern}. Ordered from
 * smallest to largest so the search returns the highest-resolution mapping
 * that still represents the requested period exactly. See ATmega640/1280/
 * 2560 datasheet section 17.11.2, table 17-6 "Clock Select Bit Description".
 */
struct prescaler_entry
{
    uint16_t divider;
    uint8_t  cs_bits;
};

static const struct prescaler_entry PRESCALER_TABLE[] = {
    {1U, 0x01U},
    {8U, 0x02U},
    {64U, 0x03U},
    {256U, 0x04U},
    {1024U, 0x05U},
};

#define PRESCALER_COUNT  (sizeof(PRESCALER_TABLE) / sizeof(PRESCALER_TABLE[0]))

/* OCR1A is 16-bit, so the maximum representable count is 65536 (with OCR1A
 * itself holding N-1). */
#define TIMER1_MAX_COUNT 65536UL

/* ========================================================================== */

/* Single-instance state: TMR1 is one physical peripheral. */
static struct
{
    tick_source_callback_t callback;
    void*                  callback_context;
    volatile uint32_t      total_ticks;
    volatile uint32_t      pending_ticks;
} state = {NULL, NULL, 0UL, 0UL};

/* ========================================================================== */

static int8_t solve_prescaler(
    uint32_t period_ms, uint8_t* cs_bits_out, uint16_t* ocr1a_out)
{
    /* Find the smallest prescaler p such that
     *     F_CPU * period_ms == 1000 * p * N,   1 <= N <= 65536.
     * uint64_t intermediate: F_CPU * period_ms can exceed 2^32 for periods
     * above ~268 ms at 16 MHz. */
    for (uint8_t i = 0U; i < PRESCALER_COUNT; ++i)
    {
        uint64_t divisor   = 1000ULL * (uint64_t)PRESCALER_TABLE[i].divider;
        uint64_t numerator = (uint64_t)F_CPU * (uint64_t)period_ms;

        if ((numerator % divisor) != 0ULL)
        {
            continue;
        }

        uint64_t ticks = numerator / divisor;
        if (ticks < 1ULL || ticks > (uint64_t)TIMER1_MAX_COUNT)
        {
            continue;
        }

        *cs_bits_out = PRESCALER_TABLE[i].cs_bits;
        *ocr1a_out   = (uint16_t)(ticks - 1ULL);
        return 0;
    }
    return -EINVAL;
}

/* ========================================================================== */

static int8_t tick_source_initialize(
    struct tick_source* self, uint32_t period_ms)
{
    if (self == NULL)
    {
        return -EFAULT;
    }
    if (period_ms == 0UL)
    {
        return -EINVAL;
    }

    uint8_t  cs_bits;
    uint16_t ocr1a_val;
    int8_t   status = solve_prescaler(period_ms, &cs_bits, &ocr1a_val);
    if (status != 0)
    {
        return status;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        /* Stop the timer while reconfiguring. */
        TCCR1B = 0U;
        TCCR1A = 0U;
        TCNT1  = 0U;
        OCR1A  = ocr1a_val;

        /* CTC mode 4: WGM13:0 = 0100 -> WGM12 lives in TCCR1B. */
        TCCR1B = (uint8_t)((1U << WGM12) | cs_bits);

        /* Clear any latched compare-match flag, then enable OCIE1A. Writing
         * a '1' to OCF1A clears it (datasheet section 17.11.9). */
        TIFR1 = (uint8_t)(1U << OCF1A);
        TIMSK1 |= (uint8_t)(1U << OCIE1A);

        state.total_ticks   = 0UL;
        state.pending_ticks = 0UL;
    }
    return 0;
}

/* ========================================================================== */

static int8_t tick_source_set_callback(
    struct tick_source* self, tick_source_callback_t callback, void* context)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    /* Update pointer and context together so the ISR never sees a callback
     * paired with a stale context. */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        state.callback         = callback;
        state.callback_context = context;
    }
    return 0;
}

/* ========================================================================== */

static bool tick_source_consume_tick(struct tick_source* self)
{
    if (self == NULL)
    {
        return false;
    }

    bool had_tick = false;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (state.pending_ticks > 0UL)
        {
            state.pending_ticks--;
            had_tick = true;
        }
    }
    return had_tick;
}

/* ========================================================================== */

static uint32_t tick_source_get_ticks(struct tick_source* self)
{
    if (self == NULL)
    {
        return 0UL;
    }

    uint32_t value;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        value = state.total_ticks;
    }
    return value;
}

/* ========================================================================== */

ISR(TIMER1_COMPA_vect)
{
    state.total_ticks++;
    state.pending_ticks++;

    if (state.callback != NULL)
    {
        state.callback(state.callback_context);
    }
}

/* ========================================================================== */

const struct tick_source_ops atmegaxx0_1_timer1_tick_source_ops = {
    .initialize   = tick_source_initialize,
    .set_callback = tick_source_set_callback,
    .consume_tick = tick_source_consume_tick,
    .get_ticks    = tick_source_get_ticks,
};

/* ========================================================================== */
