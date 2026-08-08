#include "../inc/atmegaxx0_1_spi.h"

/* ========================================================================== */

/* SPI pins on PORTB per section 1 / Table 13-6 of the datasheet. */
#define SPI_SS_BIT     PB0
#define SPI_SCK_BIT    PB1
#define SPI_MOSI_BIT   PB2
#define SPI_MISO_BIT   PB3

#define SPI_DUMMY_BYTE 0xFFU

/* Bounded busy-wait counter. At the slowest supported bit rate (fosc/128)
 * a byte takes ~1024 CPU cycles to shift; this counter tolerates roughly
 * a full byte time several times over before returning -ETIMEDOUT, so a
 * stuck peripheral can't hang the caller. Not derived from a system tick
 * because the ATmega driver set has no dependency on one. */
#define SPI_WAIT_ITER  16000U

/* ========================================================================== */

static spi_rx_callback_t g_spi_callback;
static void*             g_spi_callback_context;

/* ========================================================================== */

/* Selects (SPI2X, SPRx) for the smallest divider whose resulting bit rate
 * is <= target_freq_hz. Divider options per Table 21-5: 2/4/8/16/32/64/128. */
static int8_t calculate_prescaler(
    uint32_t target_freq_hz, uint8_t* spi2x_out, uint8_t* spr_out)
{
    if (target_freq_hz == 0U)
    {
        return -EINVAL;
    }
    if (target_freq_hz > (F_CPU / 2UL))
    {
        return -EINVAL;
    }

    static const struct
    {
        uint16_t divider;
        uint8_t  spi2x;
        uint8_t  spr;
    } table[] = {
        {2, 1, 0x0},   /* fosc/2   */
        {4, 0, 0x0},   /* fosc/4   */
        {8, 1, 0x1},   /* fosc/8   */
        {16, 0, 0x1},  /* fosc/16  */
        {32, 1, 0x2},  /* fosc/32  */
        {64, 0, 0x2},  /* fosc/64  */
        {128, 0, 0x3}, /* fosc/128 */
    };

    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
    {
        if ((F_CPU / table[i].divider) <= target_freq_hz)
        {
            *spi2x_out = table[i].spi2x;
            *spr_out   = table[i].spr;
            return 0;
        }
    }

    return -EINVAL;
}

/* ========================================================================== */

static int8_t wait_spif(void)
{
    for (uint16_t i = 0; i < SPI_WAIT_ITER; ++i)
    {
        if (SPSR & (1 << SPIF))
        {
            return 0;
        }
    }
    return -ETIMEDOUT;
}

/* ========================================================================== */

static int8_t spi_initialize(struct spi* self)
{
    if (self == NULL)
    {
        return -EINVAL;
    }
    if (!self->master)
    {
        return -ENOTSUP;
    }
    if (self->mode > 3U)
    {
        return -EINVAL;
    }

    uint8_t spi2x;
    uint8_t spr;
    int8_t  err = calculate_prescaler(self->frequency, &spi2x, &spr);
    if (err)
    {
        return err;
    }

    /* PRSPI defaults to 0 at reset but an application may have entered a
     * power-reduction state before init, so make it explicit. */
    PRR0 &= ~(1 << PRSPI);

    /* MOSI, SCK, SS as outputs; MISO as input. SS must be output in master
     * mode: if left as input and pulled low, the hardware clears MSTR and
     * the peripheral silently drops to slave (datasheet 21.1.2). */
    DDRB |= (1 << SPI_MOSI_BIT) | (1 << SPI_SCK_BIT) | (1 << SPI_SS_BIT);
    DDRB &= (uint8_t)~(1 << SPI_MISO_BIT);

    uint8_t spcr = (1 << SPE) | (1 << MSTR);
    if ((self->mode & 0x2U) != 0U)
    {
        spcr |= (1 << CPOL);
    }
    if ((self->mode & 0x1U) != 0U)
    {
        spcr |= (1 << CPHA);
    }
    if (!self->msb_first)
    {
        spcr |= (1 << DORD);
    }
    spcr |= (spr & 0x03U);

    SPCR = spcr;

    if (spi2x)
    {
        SPSR |= (1 << SPI2X);
    }
    else
    {
        SPSR &= (uint8_t)~(1 << SPI2X);
    }

    self->was_initialized = true;
    return 0;
}

/* ========================================================================== */

static int8_t spi_transmit(
    const struct spi* self, const uint8_t* buffer, size_t size)
{
    (void)self;
    if (buffer == NULL)
    {
        return -EFAULT;
    }
    if (size == 0U)
    {
        return -EINVAL;
    }
    for (size_t i = 0; i < size; ++i)
    {
        SPDR       = buffer[i];
        int8_t err = wait_spif();
        if (err)
        {
            return err;
        }
        /* Reading SPDR clears SPIF and drains the received byte we don't
         * need, preventing a stale byte from being returned by a later
         * receive-side op. */
        (void)SPDR;
    }
    return 0;
}

/* ========================================================================== */

static int8_t spi_receive(const struct spi* self, uint8_t* byte)
{
    (void)self;
    if (byte == NULL)
    {
        return -EFAULT;
    }
    SPDR       = SPI_DUMMY_BYTE;
    int8_t err = wait_spif();
    if (err)
    {
        return err;
    }
    *byte = SPDR;
    return 0;
}

/* ========================================================================== */

static int8_t spi_transfer(
    const struct spi* self,
    const uint8_t*    tx_buffer,
    uint8_t*          rx_buffer,
    size_t            size)
{
    (void)self;
    if (tx_buffer == NULL || rx_buffer == NULL)
    {
        return -EFAULT;
    }
    if (size == 0U)
    {
        return -EINVAL;
    }
    for (size_t i = 0; i < size; ++i)
    {
        SPDR       = tx_buffer[i];
        int8_t err = wait_spif();
        if (err)
        {
            return err;
        }
        rx_buffer[i] = SPDR;
    }
    return 0;
}

/* ========================================================================== */

static int8_t spi_set_cs(const struct spi* self, bool active)
{
    (void)self;
    (void)active;
    /* CS is software-managed by the caller via GPIO. The driver does not
     * know which pin is wired as CS. */
    return -ENOTSUP;
}

/* ========================================================================== */

static int8_t spi_set_frequency(const struct spi* self, uint32_t frequency)
{
    (void)self;
    uint8_t spi2x;
    uint8_t spr;
    int8_t  err = calculate_prescaler(frequency, &spi2x, &spr);
    if (err)
    {
        return err;
    }
    SPCR = (uint8_t)((SPCR & ~0x03U) | (spr & 0x03U));
    if (spi2x)
    {
        SPSR |= (1 << SPI2X);
    }
    else
    {
        SPSR &= (uint8_t)~(1 << SPI2X);
    }
    return 0;
}

/* ========================================================================== */

static int8_t spi_set_rx_callback(
    const struct spi* self, spi_rx_callback_t callback, void* callback_context)
{
    (void)self;
    if (callback == NULL)
    {
        return -EFAULT;
    }
    g_spi_callback         = callback;
    g_spi_callback_context = callback_context;
    return 0;
}

/* ========================================================================== */

static int8_t spi_enable_rx_interrupt(const struct spi* self, bool enable)
{
    (void)self;
    if (enable)
    {
        SPCR |= (1 << SPIE);
    }
    else
    {
        SPCR &= (uint8_t)~(1 << SPIE);
    }
    return 0;
}

/* ========================================================================== */

static int8_t spi_flush_tx(const struct spi* self)
{
    (void)self;
    /* Single-buffered TX: wait for any in-flight byte to finish shifting. */
    if (SPSR & (1 << SPIF))
    {
        return 0;
    }
    return wait_spif();
}

/* ========================================================================== */

static int8_t spi_flush_rx(const struct spi* self)
{
    (void)self;
    /* Reading SPSR then SPDR clears SPIF and drains the RX buffer. */
    (void)SPSR;
    (void)SPDR;
    return 0;
}

/* ========================================================================== */

const struct spi_ops atmegaxx0_1_spi_ops = {
    .initialize          = spi_initialize,
    .transmit            = spi_transmit,
    .receive             = spi_receive,
    .transfer            = spi_transfer,
    .set_cs              = spi_set_cs,
    .set_frequency       = spi_set_frequency,
    .set_rx_callback     = spi_set_rx_callback,
    .enable_rx_interrupt = spi_enable_rx_interrupt,
    .flush_tx            = spi_flush_tx,
    .flush_rx            = spi_flush_rx,
};

/* ========================================================================== */

ISR(SPI_STC_vect)
{
    uint8_t data = SPDR;
    if (g_spi_callback)
    {
        g_spi_callback(g_spi_callback_context, data);
    }
}

/* ========================================================================== */
