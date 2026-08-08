#include "../inc/atmegaxx0_1_gpio.h"

/* ========================================================================== */

/* Port register triples (PORTx / DDRx / PINx) indexed by port number, where
 * port = id / 8. AVR has no PORTI, so index 8 is reserved for PORTJ. Absent
 * ports on 64-pin variants (1281/2561) are wired to NULL and detected at
 * runtime. avr/io.h defines PORTx macros only for the ports present on the
 * selected target, so the #ifdef guards keep the table portable across the
 * family. */
static volatile uint8_t* const port_regs[] = {
    &PORTA,
    &PORTB,
    &PORTC,
    &PORTD,
    &PORTE,
    &PORTF,
    &PORTG,
#ifdef PORTH
    &PORTH,
#else
    NULL,
#endif
#ifdef PORTJ
    &PORTJ,
#else
    NULL,
#endif
#ifdef PORTK
    &PORTK,
#else
    NULL,
#endif
#ifdef PORTL
    &PORTL,
#else
    NULL,
#endif
};

static volatile uint8_t* const ddr_regs[] = {
    &DDRA,
    &DDRB,
    &DDRC,
    &DDRD,
    &DDRE,
    &DDRF,
    &DDRG,
#ifdef DDRH
    &DDRH,
#else
    NULL,
#endif
#ifdef DDRJ
    &DDRJ,
#else
    NULL,
#endif
#ifdef DDRK
    &DDRK,
#else
    NULL,
#endif
#ifdef DDRL
    &DDRL,
#else
    NULL,
#endif
};

static volatile uint8_t* const pin_regs[] = {
    &PINA,
    &PINB,
    &PINC,
    &PIND,
    &PINE,
    &PINF,
    &PING,
#ifdef PINH
    &PINH,
#else
    NULL,
#endif
#ifdef PINJ
    &PINJ,
#else
    NULL,
#endif
#ifdef PINK
    &PINK,
#else
    NULL,
#endif
#ifdef PINL
    &PINL,
#else
    NULL,
#endif
};

#define PORT_COUNT (sizeof(port_regs) / sizeof(port_regs[0]))

/* ========================================================================== */

static inline uint8_t get_port(uint8_t id)
{
    return (uint8_t)(id / 8U);
}

static inline uint8_t get_bit(uint8_t id)
{
    return (uint8_t)(id % 8U);
}

/* ========================================================================== */

static int8_t gpio_initialize(struct gpio* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    uint8_t port = get_port(self->id);
    uint8_t bit  = get_bit(self->id);

    if (port >= PORT_COUNT || ddr_regs[port] == NULL)
    {
        return -EINVAL;
    }

    /* GPIO_ANALOG on AVR is just an input with the digital buffer disabled
     * elsewhere (DIDRn, owned by the ADC driver). Here we treat it as input
     * with no pull-up. */
    if (self->type == GPIO_ANALOG || self->direction == GPIO_INPUT)
    {
        *ddr_regs[port] &= (uint8_t)~(1U << bit);
        *port_regs[port] &= (uint8_t)~(1U << bit);
    }
    else
    {
        *ddr_regs[port] |= (uint8_t)(1U << bit);
    }

    self->was_initialized = true;
    return 0;
}

/* ========================================================================== */

static int8_t gpio_set_direction(
    struct gpio* self, enum gpio_direction direction)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    uint8_t port = get_port(self->id);
    uint8_t bit  = get_bit(self->id);

    if (port >= PORT_COUNT || ddr_regs[port] == NULL)
    {
        return -EINVAL;
    }

    if (direction == GPIO_OUTPUT)
    {
        *ddr_regs[port] |= (uint8_t)(1U << bit);
    }
    else
    {
        *ddr_regs[port] &= (uint8_t)~(1U << bit);
    }

    self->direction = direction;
    return 0;
}

/* ========================================================================== */

static int8_t gpio_set_state(const struct gpio* self, bool state)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    uint8_t port = get_port(self->id);
    uint8_t bit  = get_bit(self->id);

    if (port >= PORT_COUNT || port_regs[port] == NULL)
    {
        return -EINVAL;
    }

    if (state)
    {
        *port_regs[port] |= (uint8_t)(1U << bit);
    }
    else
    {
        *port_regs[port] &= (uint8_t)~(1U << bit);
    }

    return 0;
}

/* ========================================================================== */

static int8_t gpio_get_state(const struct gpio* self, bool* state)
{
    if (self == NULL || state == NULL)
    {
        return -EFAULT;
    }

    uint8_t port = get_port(self->id);
    uint8_t bit  = get_bit(self->id);

    if (port >= PORT_COUNT || pin_regs[port] == NULL)
    {
        return -EINVAL;
    }

    *state = (*pin_regs[port] & (1U << bit)) != 0U;
    return 0;
}

/* ========================================================================== */

static int8_t gpio_toggle(const struct gpio* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    uint8_t port = get_port(self->id);
    uint8_t bit  = get_bit(self->id);

    if (port >= PORT_COUNT || pin_regs[port] == NULL)
    {
        return -EINVAL;
    }

    /* On AVR, writing a 1 to a PINx bit atomically toggles the matching
     * PORTx bit (single SBI instruction). */
    *pin_regs[port] = (uint8_t)(1U << bit);
    return 0;
}

/* ========================================================================== */

const struct gpio_ops atmegaxx0_1_gpio_ops = {
    .initialize    = gpio_initialize,
    .set_direction = gpio_set_direction,
    .set_state     = gpio_set_state,
    .get_state     = gpio_get_state,
    .toggle        = gpio_toggle,
};

/* ========================================================================== */
