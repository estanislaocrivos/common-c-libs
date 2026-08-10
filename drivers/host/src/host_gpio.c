#include "../inc/host_gpio.h"

/* ========================================================================== */

/* Virtual pin state, indexed by struct gpio::id. There is no real hardware
 * to read back from, so set_state/toggle write here and get_state reads it
 * back from here. */
static bool pin_state[HOST_GPIO_PIN_COUNT];

/* ========================================================================== */

static int8_t gpio_initialize(struct gpio* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    if (self->id >= HOST_GPIO_PIN_COUNT)
    {
        return -EINVAL;
    }

    pin_state[self->id] = false;

    printf(
        "[host_gpio] pin %u: initialized as %s\n",
        self->id,
        self->direction == GPIO_OUTPUT ? "output" : "input");

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

    if (self->id >= HOST_GPIO_PIN_COUNT)
    {
        return -EINVAL;
    }

    printf(
        "[host_gpio] pin %u: direction -> %s\n",
        self->id,
        direction == GPIO_OUTPUT ? "output" : "input");

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

    if (self->id >= HOST_GPIO_PIN_COUNT)
    {
        return -EINVAL;
    }

    pin_state[self->id] = state;

    printf("[host_gpio] pin %u: set %s\n", self->id, state ? "high" : "low");

    return 0;
}

/* ========================================================================== */

static int8_t gpio_get_state(const struct gpio* self, bool* state)
{
    if (self == NULL || state == NULL)
    {
        return -EFAULT;
    }

    if (self->id >= HOST_GPIO_PIN_COUNT)
    {
        return -EINVAL;
    }

    *state = pin_state[self->id];
    return 0;
}

/* ========================================================================== */

static int8_t gpio_toggle(const struct gpio* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    if (self->id >= HOST_GPIO_PIN_COUNT)
    {
        return -EINVAL;
    }

    pin_state[self->id] = !pin_state[self->id];

    printf(
        "[host_gpio] pin %u: toggled -> %s\n",
        self->id,
        pin_state[self->id] ? "high" : "low");

    return 0;
}

/* ========================================================================== */

const struct gpio_ops host_gpio_ops = {
    .initialize    = gpio_initialize,
    .set_direction = gpio_set_direction,
    .set_state     = gpio_set_state,
    .get_state     = gpio_get_state,
    .toggle        = gpio_toggle,
};

/* ========================================================================== */
