#include "../inc/atmegaxx0_1_adc.h"

/* ========================================================================== */

#ifdef DIDR2
#define ADC_CHANNEL_COUNT 16U
#else
#define ADC_CHANNEL_COUNT 8U
#endif

#define ADC_TARGET_MAX_CLOCK_HZ 200000UL

/* ========================================================================== */

/* One physical ADC peripheral: the instance currently bound to the
 * conversion-complete interrupt, or NULL if none. */
static struct adc* interrupt_adc_instance = NULL;

/* ========================================================================== */

static uint8_t select_prescaler_bits(void)
{
    /* Recommended ADC clock is 50-200kHz for full 10-bit accuracy. Pick the
     * largest prescaler (best noise immunity) that still meets that ceiling
     * for the application's F_CPU. */
    if (F_CPU / 128UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x07U;
    }
    if (F_CPU / 64UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x06U;
    }
    if (F_CPU / 32UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x05U;
    }
    if (F_CPU / 16UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x04U;
    }
    if (F_CPU / 8UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x03U;
    }
    if (F_CPU / 4UL <= ADC_TARGET_MAX_CLOCK_HZ)
    {
        return 0x02U;
    }
    return 0x01U;
}

/* ========================================================================== */

static int8_t select_vref_bits(uint8_t volt_reference, uint8_t* refs_bits)
{
    switch (volt_reference)
    {
        case ADC_VREF_VDD:
            *refs_bits = 0x01U; /* AVCC with external cap at AREF */
            return 0;
        case ADC_VREF_EXTERNAL:
            *refs_bits = 0x00U; /* AREF, internal Vref off */
            return 0;
        case ADC_VREF_FVR_1024:
            *refs_bits = 0x03U; /* internal 1.1V reference */
            return 0;
        default:
            return -ENOTSUP;
    }
}

/* ========================================================================== */

static void select_channel(uint8_t channel)
{
    /* MUX4:0 selects the channel within its half (0-7); MUX5 (ADCSRB, only
     * present on family members with channels 8-15) picks which half. */
    ADMUX = (uint8_t)((ADMUX & 0xE0U) | (channel & 0x07U));

#ifdef MUX5
    if (channel >= 8U)
    {
        ADCSRB |= (1U << MUX5);
    }
    else
    {
        ADCSRB &= (uint8_t)~(1U << MUX5);
    }
#endif

    if (channel < 8U)
    {
        DIDR0 |= (uint8_t)(1U << channel);
    }
#ifdef DIDR2
    else
    {
        DIDR2 |= (uint8_t)(1U << (channel - 8U));
    }
#endif
}

/* ========================================================================== */

static int8_t adc_initialize(struct adc* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    if (self->resolution != 10U)
    {
        return -ENOTSUP;
    }

    uint8_t refs_bits;
    int8_t  status = select_vref_bits(self->volt_reference, &refs_bits);
    if (status != 0)
    {
        return status;
    }

    ADMUX  = (uint8_t)(refs_bits << REFS0);
    ADCSRA = (uint8_t)((1U << ADEN) | select_prescaler_bits());

    self->was_initialized = true;
    return 0;
}

/* ========================================================================== */

static int8_t adc_read(struct adc* self, uint8_t channel, uint32_t* value)
{
    if (self == NULL || value == NULL)
    {
        return -EFAULT;
    }
    if (!self->was_initialized)
    {
        return -EPERM;
    }
    if (channel >= ADC_CHANNEL_COUNT)
    {
        return -EINVAL;
    }

    select_channel(channel);

    ADCSRA |= (1U << ADSC);
    while ((ADCSRA & (1U << ADSC)) != 0U)
    {
        /* blocking: wait for the single conversion to complete */
    }

    *value = ADC;
    return 0;
}

/* ========================================================================== */

static int8_t adc_start_conversion(struct adc* self, uint8_t channel)
{
    if (self == NULL)
    {
        return -EFAULT;
    }
    if (!self->was_initialized)
    {
        return -EPERM;
    }
    if (channel >= ADC_CHANNEL_COUNT)
    {
        return -EINVAL;
    }

    select_channel(channel);
    ADCSRA |= (1U << ADSC);
    return 0;
}

/* ========================================================================== */

static int8_t adc_stop_conversion(struct adc* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    ADCSRA &= (uint8_t)~(1U << ADSC);
    return 0;
}

/* ========================================================================== */

static int8_t adc_is_conversion_complete(struct adc* self, bool* complete)
{
    if (self == NULL || complete == NULL)
    {
        return -EFAULT;
    }

    *complete = (ADCSRA & (1U << ADSC)) == 0U;
    return 0;
}

/* ========================================================================== */

static int8_t adc_get_value(struct adc* self, uint32_t* value)
{
    if (self == NULL || value == NULL)
    {
        return -EFAULT;
    }

    *value = ADC;
    return 0;
}

/* ========================================================================== */

static int8_t adc_set_conversion_callback(
    struct adc*               self,
    adc_conversion_callback_t callback,
    void*                     callback_context)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    self->callback         = callback;
    self->callback_context = callback_context;
    return 0;
}

/* ========================================================================== */

static void adc_enable_conv_interrupt(struct adc* self, bool enable)
{
    if (self == NULL)
    {
        return;
    }

    interrupt_adc_instance = enable ? self : NULL;

    if (enable)
    {
        ADCSRA |= (1U << ADIE);
    }
    else
    {
        ADCSRA &= (uint8_t)~(1U << ADIE);
    }
}

/* ========================================================================== */

ISR(ADC_vect)
{
    if (interrupt_adc_instance != NULL
        && interrupt_adc_instance->callback != NULL)
    {
        interrupt_adc_instance->callback(
            interrupt_adc_instance->callback_context, ADC);
    }
}

/* ========================================================================== */

const struct adc_ops atmegaxx0_1_adc_ops = {
    .initialize              = adc_initialize,
    .read                    = adc_read,
    .start_conversion        = adc_start_conversion,
    .stop_conversion         = adc_stop_conversion,
    .is_conversion_complete  = adc_is_conversion_complete,
    .get_value               = adc_get_value,
    .set_conversion_callback = adc_set_conversion_callback,
    .enable_conv_interrupt   = adc_enable_conv_interrupt,
};

/* ========================================================================== */
