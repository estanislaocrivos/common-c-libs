#ifndef ATMEGAXX0_1_I2C_H
#define ATMEGAXX0_1_I2C_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief i2c_ops implementation for the single TWI (2-wire) peripheral of
 *        the ATmega640/1280/1281/2560/2561 family. Pass &atmegaxx0_1_twi_ops
 *        to your struct i2c to bind it.
 *
 * @note  Only master mode is supported. struct i2c::master must be true;
 *        initialize() returns -ENOTSUP otherwise.
 *
 * @note  Only 7-bit addressing is supported. address_mode must be
 *        I2C_ADDR_7BIT.
 *
 * @note  Polling-based (no interrupts). Each op busy-waits on TWINT bounded
 *        by an iteration counter, so a stuck bus returns -ETIMEDOUT instead
 *        of hanging.
 *
 * @note  Pin configuration is automatic: setting TWEN in TWCR overrides
 *        PD0/PD1 to SCL/SDA regardless of DDRD. External pull-ups on SDA
 *        and SCL are still required (the AVR internal pull-ups are not
 *        enabled by this driver).
 */
extern const struct i2c_ops atmegaxx0_1_twi_ops;

/* ========================================================================== */

#endif  // ATMEGAXX0_1_I2C_H
