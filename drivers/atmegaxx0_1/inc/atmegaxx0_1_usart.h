#ifndef ATMEGAXX0_1_USART_H
#define ATMEGAXX0_1_USART_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief serial_ops implementations for the USART instances of the
 *        ATmega640/1280/1281/2560/2561 family. Pass &atmegaxx0_1_usartN_ops
 *        to SERIAL_DEFINE to bind a serial port to the desired peripheral.
 *
 * @note  USART0 and USART1 are present on every family member. USART2 and
 *        USART3 exist only on the 100-pin variants (640/1280/2560) and are
 *        conditionally compiled based on target-specific macros defined by
 *        avr-libc. Referencing atmegaxx0_1_usart2_ops or _usart3_ops on a
 *        64-pin variant will fail at link time.
 *
 * @note  Supported frame formats: 5-9 data bits, N/E/O parity, 1 or 2
 *        stop bits. Async normal mode (U2X=0) only. Any other combination
 *        makes initialize() return -ENOTSUP.
 */
extern const struct serial_ops atmegaxx0_1_usart0_ops;
extern const struct serial_ops atmegaxx0_1_usart1_ops;

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__)
extern const struct serial_ops atmegaxx0_1_usart2_ops;
extern const struct serial_ops atmegaxx0_1_usart3_ops;
#endif

/* ========================================================================== */

#endif  // ATMEGAXX0_1_USART_H
