#ifndef ATMEGAXX0_1_SPI_H
#define ATMEGAXX0_1_SPI_H

/* ========================================================================== */

#include "config.h"

/* ========================================================================== */

/**
 * @brief spi_ops implementation for the single SPI peripheral of the
 *        ATmega640/1280/1281/2560/2561 family. Pass &atmegaxx0_1_spi_ops
 *        to your struct spi to bind it.
 *
 * @note  Only master mode is supported. struct spi::master must be true;
 *        initialize() returns -ENOTSUP otherwise.
 *
 * @note  Chip Select is software-managed by the caller via GPIO. The set_cs
 *        op returns -ENOTSUP because the driver has no knowledge of which
 *        pin acts as CS. The hardware SS pin (PB0) IS configured as output
 *        by initialize() regardless — per datasheet section 21.1.2, if SS
 *        is left as input and pulled low while MSTR is set, the peripheral
 *        silently drops to slave mode.
 *
 * @note  Pin configuration for MOSI (PB2), SCK (PB1) and SS (PB0) as
 *        outputs and MISO (PB3) as input is performed by initialize().
 */
extern const struct spi_ops atmegaxx0_1_spi_ops;

/* ========================================================================== */

#endif  // ATMEGAXX0_1_SPI_H
