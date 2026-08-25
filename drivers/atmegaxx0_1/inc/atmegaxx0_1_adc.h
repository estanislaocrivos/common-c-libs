#ifndef ATMEGAXX0_1_ADC_H
#define ATMEGAXX0_1_ADC_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief adc_ops implementation for the ADC peripheral of the
 *        ATmega640/1280/1281/2560/2561 family. Pass &atmegaxx0_1_adc_ops to
 *        ADC_DEFINE as the ops_ptr argument.
 *
 * @note  Only single 10-bit conversions are supported; `resolution` must be
 *        10. Channels 0-7 (ADC0-7, PORTF) are available on every family
 *        member; channels 8-15 (ADC8-15, PORTK) only exist on the 100-pin
 *        variants (640/1280/2560) and return -EINVAL on the 64-pin ones
 *        (1281/2561). `port_id` and `sampling_time` are unused -- the ADC
 *        clock prescaler is chosen automatically from F_CPU to stay within
 *        the datasheet's recommended 50-200kHz range for full 10-bit
 *        accuracy.
 *
 * @note  `volt_reference` accepts ADC_VREF_VDD (AVCC), ADC_VREF_EXTERNAL
 *        (AREF) and ADC_VREF_FVR_1024 (the family's internal 1.1V
 *        reference). Any other value returns -ENOTSUP.
 *
 * @note  Selecting a channel disables that pin's digital input buffer
 *        (DIDR0/DIDR2), matching the GPIO driver's note that
 *        gpio_type::GPIO_ANALOG leaves buffer ownership to this driver.
 *
 * @note  There is one physical ADC peripheral, so only one struct adc
 *        instance should be interrupt-driven (enable_conv_interrupt) at a
 *        time -- the most recent call wins.
 */
extern const struct adc_ops atmegaxx0_1_adc_ops;

/* ========================================================================== */

#endif  // ATMEGAXX0_1_ADC_H
