#ifndef BOARD_H
#define BOARD_H

/* ========================================================================== */

/**
 * @brief Board binding for the Arduino Mega 2560 (ATmega2560).
 *
 * Maps the generic PLATFORM_*_OPS macros to this board's concrete driver
 * ops instances, so application code can bind embedded-hal structs (via
 * GPIO_DEFINE, SERIAL_DEFINE, etc.) without knowing which MCU it targets.
 * Only #include "board.h" — swapping boards/toolchains is enough to pick a
 * different set of ops for the same app source.
 *
 * @note  Clock frequency, pin assignments and any other app-specific
 *        configuration are NOT defined here — that stays the application's
 *        responsibility.
 */

/* ========================================================================== */

#include "atmegaxx0_1.h"

/* ========================================================================== */

#define PLATFORM_ADC_OPS    atmegaxx0_1_adc_ops
#define PLATFORM_GPIO_OPS   atmegaxx0_1_gpio_ops
#define PLATFORM_I2C_OPS    atmegaxx0_1_twi_ops
#define PLATFORM_SPI_OPS    atmegaxx0_1_spi_ops
#define PLATFORM_USART0_OPS atmegaxx0_1_usart0_ops
#define PLATFORM_USART1_OPS atmegaxx0_1_usart1_ops

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__)
#define PLATFORM_USART2_OPS atmegaxx0_1_usart2_ops
#define PLATFORM_USART3_OPS atmegaxx0_1_usart3_ops
#endif

/* ========================================================================== */

#endif  // BOARD_H
