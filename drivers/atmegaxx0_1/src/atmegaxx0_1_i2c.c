#include "../inc/atmegaxx0_1_i2c.h"

/* ========================================================================== */

/* TWI status code values with the prescaler bits (TWPS1:0) already masked
 * out. Datasheet section 24.7 tables 24-2..24-5. */
#define TWI_STATUS_MASK         0xF8U

#define TWI_STATUS_START        0x08U
#define TWI_STATUS_RSTART       0x10U
#define TWI_STATUS_MT_SLA_ACK   0x18U
#define TWI_STATUS_MT_SLA_NACK  0x20U
#define TWI_STATUS_MT_DATA_ACK  0x28U
#define TWI_STATUS_MT_DATA_NACK 0x30U
#define TWI_STATUS_MR_SLA_ACK   0x40U
#define TWI_STATUS_MR_SLA_NACK  0x48U
#define TWI_STATUS_MR_DATA_ACK  0x50U
#define TWI_STATUS_MR_DATA_NACK 0x58U

/* Datasheet recommends TWBR >= 10 for reliable master-mode operation. */
#define TWI_TWBR_MIN            10U
#define TWI_TWBR_MAX            255U

/* Bounded busy-wait counter for the TWINT flag. Each iteration is a handful
 * of CPU cycles; 200 000 iterations tolerates roughly a few hundred ms at
 * 16 MHz, generous enough for slaves that stretch SCL but bounded so a
 * missing pull-up or a stuck bus returns -ETIMEDOUT instead of hanging. */
#define TWI_WAIT_ITER           200000UL

/* ========================================================================== */

static inline int8_t wait_twint(void)
{
    for (uint32_t i = 0; i < TWI_WAIT_ITER; ++i)
    {
        if (TWCR & (1U << TWINT))
        {
            return 0;
        }
    }
    return -ETIMEDOUT;
}

static inline uint8_t twi_status(void)
{
    return (uint8_t)(TWSR & TWI_STATUS_MASK);
}

/* ========================================================================== */

/* Solve TWBR/TWPS for the requested SCL frequency:
 *     f_scl = F_CPU / (16 + 2 * TWBR * 4^TWPS)
 * Iterate prescalers from smallest to largest and pick the first that
 * yields TWBR within [10, 255]. Fails with -EINVAL if no combination fits. */
static int8_t calculate_bit_rate(
    uint32_t target_scl_hz, uint8_t* twbr_out, uint8_t* twps_out)
{
    if (target_scl_hz == 0U)
    {
        return -EINVAL;
    }
    if ((F_CPU / target_scl_hz) < 16U)
    {
        return -EINVAL;
    }

    static const struct
    {
        uint8_t  twps;
        uint16_t factor; /* 4^TWPS */
    } table[] = {
        {0, 1},
        {1, 4},
        {2, 16},
        {3, 64},
    };

    uint32_t divider = F_CPU / target_scl_hz;
    if (divider <= 16U)
    {
        return -EINVAL;
    }
    uint32_t numerator = (divider - 16U) / 2U;

    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); ++i)
    {
        uint32_t twbr = numerator / table[i].factor;
        if (twbr >= TWI_TWBR_MIN && twbr <= TWI_TWBR_MAX)
        {
            *twbr_out = (uint8_t)twbr;
            *twps_out = table[i].twps;
            return 0;
        }
    }
    return -EINVAL;
}

/* ========================================================================== */

/* Force a STOP condition on the bus. Used after every error path so the
 * slave and any bystander masters see the transaction end cleanly. */
static void send_stop(void)
{
    TWCR = (uint8_t)((1U << TWINT) | (1U << TWEN) | (1U << TWSTO));
    /* TWSTO is cleared by hardware once the STOP has been executed. Wait
     * for that so back-to-back transactions don't overlap. */
    while (TWCR & (1U << TWSTO))
    {
    }
}

/* ========================================================================== */

static int8_t send_start(uint8_t expected_status)
{
    TWCR       = (uint8_t)((1U << TWINT) | (1U << TWSTA) | (1U << TWEN));
    int8_t err = wait_twint();
    if (err)
    {
        return err;
    }
    if (twi_status() != expected_status)
    {
        return -EIO;
    }
    return 0;
}

/* ========================================================================== */

/* Send SLA+R/W. Datasheet §24.7.1: after writing TWDR, clear TWINT with
 * TWSTA=0 to actually shift the address out; wait for TWINT and verify
 * the acknowledgement matches the direction. */
static int8_t send_addr(uint16_t slave_addr, uint8_t rw_bit)
{
    TWDR = (uint8_t)((slave_addr << 1) | (rw_bit & 0x01U));
    TWCR = (uint8_t)((1U << TWINT) | (1U << TWEN));

    int8_t err = wait_twint();
    if (err)
    {
        return err;
    }

    uint8_t status = twi_status();
    if (rw_bit == 0U)
    {
        /* SLA+W */
        if (status == TWI_STATUS_MT_SLA_ACK)
        {
            return 0;
        }
        if (status == TWI_STATUS_MT_SLA_NACK)
        {
            return -ENXIO;
        }
    }
    else
    {
        /* SLA+R */
        if (status == TWI_STATUS_MR_SLA_ACK)
        {
            return 0;
        }
        if (status == TWI_STATUS_MR_SLA_NACK)
        {
            return -ENXIO;
        }
    }
    return -EIO;
}

/* ========================================================================== */

static int8_t send_data(uint8_t byte)
{
    TWDR = byte;
    TWCR = (uint8_t)((1U << TWINT) | (1U << TWEN));

    int8_t err = wait_twint();
    if (err)
    {
        return err;
    }

    uint8_t status = twi_status();
    if (status == TWI_STATUS_MT_DATA_ACK)
    {
        return 0;
    }
    if (status == TWI_STATUS_MT_DATA_NACK)
    {
        return -EIO;
    }
    return -EIO;
}

/* ========================================================================== */

/* Receive one byte in MR mode. If ack_next is true, drive an ACK after
 * the byte to keep the slave streaming; otherwise NACK to signal end. */
static int8_t receive_data(uint8_t* byte, bool ack_next)
{
    uint8_t cr = (uint8_t)((1U << TWINT) | (1U << TWEN));
    if (ack_next)
    {
        cr |= (uint8_t)(1U << TWEA);
    }
    TWCR = cr;

    int8_t err = wait_twint();
    if (err)
    {
        return err;
    }

    uint8_t status          = twi_status();
    uint8_t expected_status = ack_next ? TWI_STATUS_MR_DATA_ACK :
                                         TWI_STATUS_MR_DATA_NACK;
    if (status != expected_status)
    {
        return -EIO;
    }

    *byte = TWDR;
    return 0;
}

/* ========================================================================== */

static int8_t twi_initialize(struct i2c* self)
{
    if (self == NULL)
    {
        return -EINVAL;
    }
    if (!self->master)
    {
        return -ENOTSUP;
    }
    if (self->address_mode != I2C_ADDR_7BIT)
    {
        return -ENOTSUP;
    }

    uint8_t twbr;
    uint8_t twps;
    int8_t  err = calculate_bit_rate(self->frequency, &twbr, &twps);
    if (err)
    {
        return err;
    }

    /* Enable the module's clock in case an application left it in a power-
     * reduction state. PRTWI is bit 7 of PRR0 (datasheet §11 / §24.2). */
    PRR0 &= (uint8_t)~(1U << PRTWI);

    TWBR = twbr;
    TWSR = (uint8_t)(twps & 0x03U);
    TWCR = (uint8_t)(1U << TWEN);

    self->was_initialized = true;
    return 0;
}

/* ========================================================================== */

static int8_t twi_write_raw(
    const struct i2c* self, const uint8_t* tx_payload, size_t tx_payload_size)
{
    if (self == NULL || tx_payload == NULL)
    {
        return -EFAULT;
    }
    if (tx_payload_size == 0U)
    {
        return -EINVAL;
    }

    int8_t err = send_start(TWI_STATUS_START);
    if (err)
    {
        return err;
    }

    err = send_addr(self->slave_address, 0U);
    if (err)
    {
        send_stop();
        return err;
    }

    for (size_t i = 0; i < tx_payload_size; ++i)
    {
        err = send_data(tx_payload[i]);
        if (err)
        {
            send_stop();
            return err;
        }
    }

    send_stop();
    return 0;
}

/* ========================================================================== */

static int8_t twi_write(
    const struct i2c* self,
    uint8_t           reg_address,
    const uint8_t*    tx_payload,
    size_t            tx_payload_size)
{
    if (self == NULL || tx_payload == NULL)
    {
        return -EFAULT;
    }
    if (tx_payload_size == 0U)
    {
        return -EINVAL;
    }

    int8_t err = send_start(TWI_STATUS_START);
    if (err)
    {
        return err;
    }

    err = send_addr(self->slave_address, 0U);
    if (err)
    {
        send_stop();
        return err;
    }

    err = send_data(reg_address);
    if (err)
    {
        send_stop();
        return err;
    }

    for (size_t i = 0; i < tx_payload_size; ++i)
    {
        err = send_data(tx_payload[i]);
        if (err)
        {
            send_stop();
            return err;
        }
    }

    send_stop();
    return 0;
}

/* ========================================================================== */

static int8_t twi_read_raw(
    const struct i2c* self, uint8_t* rx_payload, size_t rx_payload_size)
{
    if (self == NULL || rx_payload == NULL)
    {
        return -EFAULT;
    }
    if (rx_payload_size == 0U)
    {
        return -EINVAL;
    }

    int8_t err = send_start(TWI_STATUS_START);
    if (err)
    {
        return err;
    }

    err = send_addr(self->slave_address, 1U);
    if (err)
    {
        send_stop();
        return err;
    }

    for (size_t i = 0; i < rx_payload_size; ++i)
    {
        bool ack_next = (i < rx_payload_size - 1U);
        err           = receive_data(&rx_payload[i], ack_next);
        if (err)
        {
            send_stop();
            return err;
        }
    }

    send_stop();
    return 0;
}

/* ========================================================================== */

static int8_t twi_read(
    const struct i2c* self,
    uint8_t           reg_address,
    uint8_t*          rx_payload,
    size_t            rx_payload_size)
{
    if (self == NULL || rx_payload == NULL)
    {
        return -EFAULT;
    }
    if (rx_payload_size == 0U)
    {
        return -EINVAL;
    }

    /* Write phase: SLA+W + reg_address, no STOP. */
    int8_t err = send_start(TWI_STATUS_START);
    if (err)
    {
        return err;
    }

    err = send_addr(self->slave_address, 0U);
    if (err)
    {
        send_stop();
        return err;
    }

    err = send_data(reg_address);
    if (err)
    {
        send_stop();
        return err;
    }

    /* Read phase: repeated START + SLA+R + N bytes. */
    err = send_start(TWI_STATUS_RSTART);
    if (err)
    {
        send_stop();
        return err;
    }

    err = send_addr(self->slave_address, 1U);
    if (err)
    {
        send_stop();
        return err;
    }

    for (size_t i = 0; i < rx_payload_size; ++i)
    {
        bool ack_next = (i < rx_payload_size - 1U);
        err           = receive_data(&rx_payload[i], ack_next);
        if (err)
        {
            send_stop();
            return err;
        }
    }

    send_stop();
    return 0;
}

/* ========================================================================== */

static int8_t twi_probe(const struct i2c* self)
{
    if (self == NULL)
    {
        return -EFAULT;
    }

    int8_t err = send_start(TWI_STATUS_START);
    if (err)
    {
        return err;
    }

    err = send_addr(self->slave_address, 0U);
    send_stop();
    return err;
}

/* ========================================================================== */

const struct i2c_ops atmegaxx0_1_twi_ops = {
    .initialize = twi_initialize,
    .write      = twi_write,
    .read       = twi_read,
    .write_raw  = twi_write_raw,
    .read_raw   = twi_read_raw,
    .probe      = twi_probe,
};

/* ========================================================================== */
