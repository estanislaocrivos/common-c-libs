#include "../inc/atmegaxx0_1_usart.h"

/* ========================================================================== */

/* Per-instance context. AVR USARTs don't share a uniform register layout in
 * memory the way a Cortex-M peripheral does, so instead of a single struct
 * pointer we carry direct pointers to each control/status/data register. */
struct usart_ctx
{
    volatile uint8_t*    ubrrh;
    volatile uint8_t*    ubrrl;
    volatile uint8_t*    ucsra;
    volatile uint8_t*    ucsrb;
    volatile uint8_t*    ucsrc;
    volatile uint8_t*    udr;
    serial_rx_callback_t callback;
    void*                callback_context;
};

/* ========================================================================== */

static struct usart_ctx g_usart0_ctx = {
    .ubrrh            = &UBRR0H,
    .ubrrl            = &UBRR0L,
    .ucsra            = &UCSR0A,
    .ucsrb            = &UCSR0B,
    .ucsrc            = &UCSR0C,
    .udr              = &UDR0,
    .callback         = NULL,
    .callback_context = NULL,
};

static struct usart_ctx g_usart1_ctx = {
    .ubrrh            = &UBRR1H,
    .ubrrl            = &UBRR1L,
    .ucsra            = &UCSR1A,
    .ucsrb            = &UCSR1B,
    .ucsrc            = &UCSR1C,
    .udr              = &UDR1,
    .callback         = NULL,
    .callback_context = NULL,
};

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__)
static struct usart_ctx g_usart2_ctx = {
    .ubrrh            = &UBRR2H,
    .ubrrl            = &UBRR2L,
    .ucsra            = &UCSR2A,
    .ucsrb            = &UCSR2B,
    .ucsrc            = &UCSR2C,
    .udr              = &UDR2,
    .callback         = NULL,
    .callback_context = NULL,
};

static struct usart_ctx g_usart3_ctx = {
    .ubrrh            = &UBRR3H,
    .ubrrl            = &UBRR3L,
    .ucsra            = &UCSR3A,
    .ucsrb            = &UCSR3B,
    .ucsrc            = &UCSR3C,
    .udr              = &UDR3,
    .callback         = NULL,
    .callback_context = NULL,
};
#endif

/* ========================================================================== */

/* Character-size bit values as they appear across UCSRnB (bit UCSZn2) and
 * UCSRnC (bits UCSZn1:0). Datasheet section 22.11.4 Table 22-11. */
struct char_size_bits
{
    uint8_t ucsz2;
    uint8_t ucsz10;
};

static int8_t char_size_from_bits(uint8_t data_bits, struct char_size_bits* out)
{
    switch (data_bits)
    {
        case 5:
            out->ucsz2  = 0;
            out->ucsz10 = 0;
            return 0;
        case 6:
            out->ucsz2  = 0;
            out->ucsz10 = 1;
            return 0;
        case 7:
            out->ucsz2  = 0;
            out->ucsz10 = 2;
            return 0;
        case 8:
            out->ucsz2  = 0;
            out->ucsz10 = 3;
            return 0;
        case 9:
            out->ucsz2  = 1;
            out->ucsz10 = 3;
            return 0;
        default:
            return -ENOTSUP;
    }
}

/* ========================================================================== */

static int8_t apply_frame_format(struct usart_ctx* ctx, const struct serial* s)
{
    struct char_size_bits csz;
    int8_t                err = char_size_from_bits(s->data_bits, &csz);
    if (err)
    {
        return err;
    }

    uint8_t ucsrb = *ctx->ucsrb;
    uint8_t ucsrc = 0;

    /* UCSZn2 lives in UCSRnB bit 2 (per datasheet 22.11.2). Keep RXEN/TXEN
     * bits untouched — they are set by initialize() after this call. */
    if (csz.ucsz2)
    {
        ucsrb |= (uint8_t)(1U << UCSZ02);
    }
    else
    {
        ucsrb &= (uint8_t)~(1U << UCSZ02);
    }
    ucsrc |= (uint8_t)(csz.ucsz10 << UCSZ00);

    /* Parity: UPMn1:0 in UCSRnC bits 5:4. 00=none, 10=even, 11=odd. */
    switch (s->parity)
    {
        case 'N':
            break;
        case 'E':
            ucsrc |= (uint8_t)(1U << UPM01);
            break;
        case 'O':
            ucsrc |= (uint8_t)((1U << UPM01) | (1U << UPM00));
            break;
        default:
            return -ENOTSUP;
    }

    /* Stop bits: USBSn in UCSRnC bit 3. */
    if (s->stop_bits == 2U)
    {
        ucsrc |= (uint8_t)(1U << USBS0);
    }
    else if (s->stop_bits != 1U)
    {
        return -ENOTSUP;
    }

    *ctx->ucsrb = ucsrb;
    *ctx->ucsrc = ucsrc;
    return 0;
}

/* ========================================================================== */

static int8_t initialize_ctx(struct usart_ctx* ctx, struct serial* s)
{
    if (s == NULL || s->baud_rate == 0U)
    {
        return -EINVAL;
    }

    /* Async normal mode: UBRR = fosc / (16 * baud) - 1. Reject settings the
     * 12-bit UBRR can't represent. */
    uint32_t ubrr = (F_CPU / (16UL * s->baud_rate));
    if (ubrr == 0U)
    {
        return -EINVAL;
    }
    ubrr -= 1U;
    if (ubrr > 4095U)
    {
        return -EINVAL;
    }

    *ctx->ucsra = 0U;
    *ctx->ucsrb = 0U;
    *ctx->ucsrc = 0U;
    *ctx->ubrrh = (uint8_t)(ubrr >> 8);
    *ctx->ubrrl = (uint8_t)ubrr;

    int8_t err = apply_frame_format(ctx, s);
    if (err)
    {
        return err;
    }

    *ctx->ucsrb |= (uint8_t)((1U << RXEN0) | (1U << TXEN0));

    s->was_initialized = true;
    return 0;
}

/* ========================================================================== */

static int8_t transmit_ctx(
    struct usart_ctx* ctx, const uint8_t* buffer, size_t size)
{
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
        while (!(*ctx->ucsra & (1U << UDRE0)))
        {
        }
        *ctx->udr = buffer[i];
    }
    return 0;
}

/* ========================================================================== */

static int8_t receive_ctx(struct usart_ctx* ctx, uint8_t* byte)
{
    if (byte == NULL)
    {
        return -EFAULT;
    }
    while (!(*ctx->ucsra & (1U << RXC0)))
    {
    }
    *byte = *ctx->udr;
    return 0;
}

/* ========================================================================== */

static int8_t set_rx_callback_ctx(
    struct usart_ctx*    ctx,
    serial_rx_callback_t callback,
    void*                callback_context)
{
    if (callback == NULL)
    {
        return -EFAULT;
    }
    ctx->callback         = callback;
    ctx->callback_context = callback_context;
    return 0;
}

/* ========================================================================== */

static int8_t enable_rx_interrupt_ctx(struct usart_ctx* ctx, bool enable)
{
    if (enable)
    {
        *ctx->ucsrb |= (uint8_t)(1U << RXCIE0);
    }
    else
    {
        *ctx->ucsrb &= (uint8_t)~(1U << RXCIE0);
    }
    return 0;
}

/* ========================================================================== */

static int8_t set_rx_interrupt_priority_ctx(
    struct usart_ctx* ctx, uint8_t priority)
{
    (void)ctx;
    (void)priority;
    /* AVR has fixed interrupt priorities determined by vector address (see
     * datasheet section 14). There is no NVIC to program. */
    return -ENOTSUP;
}

/* ========================================================================== */

static int8_t flush_tx_ctx(struct usart_ctx* ctx)
{
    /* Wait for the transmit shift register to drain (TXCn = 1). Write a 1
     * to clear TXCn first so we don't return early on a stale flag. */
    *ctx->ucsra |= (uint8_t)(1U << TXC0);
    while (!(*ctx->ucsra & (1U << UDRE0)))
    {
    }
    while (!(*ctx->ucsra & (1U << TXC0)))
    {
    }
    return 0;
}

/* ========================================================================== */

static int8_t flush_rx_ctx(struct usart_ctx* ctx)
{
    /* Drain the 2-frame receive FIFO. Reading UDRn pops one frame. */
    while (*ctx->ucsra & (1U << RXC0))
    {
        (void)*ctx->udr;
    }
    return 0;
}

/* ========================================================================== */

static inline void handle_rx_isr(struct usart_ctx* ctx)
{
    uint8_t data = *ctx->udr;
    if (ctx->callback != NULL)
    {
        ctx->callback(ctx->callback_context, data);
    }
}

/* ========================================================================== */

/* USART0 op wrappers */

static int8_t usart0_initialize(struct serial* self)
{
    return initialize_ctx(&g_usart0_ctx, self);
}

static int8_t usart0_transmit(
    const struct serial* self, const uint8_t* buffer, size_t size)
{
    (void)self;
    return transmit_ctx(&g_usart0_ctx, buffer, size);
}

static int8_t usart0_receive(const struct serial* self, uint8_t* byte)
{
    (void)self;
    return receive_ctx(&g_usart0_ctx, byte);
}

static int8_t usart0_set_rx_callback(
    const struct serial* self,
    serial_rx_callback_t callback,
    void*                callback_context)
{
    (void)self;
    return set_rx_callback_ctx(&g_usart0_ctx, callback, callback_context);
}

static int8_t usart0_enable_rx_interrupt(const struct serial* self, bool enable)
{
    (void)self;
    return enable_rx_interrupt_ctx(&g_usart0_ctx, enable);
}

static int8_t usart0_set_rx_interrupt_priority(
    const struct serial* self, uint8_t priority)
{
    (void)self;
    return set_rx_interrupt_priority_ctx(&g_usart0_ctx, priority);
}

static int8_t usart0_flush_tx(const struct serial* self)
{
    (void)self;
    return flush_tx_ctx(&g_usart0_ctx);
}

static int8_t usart0_flush_rx(const struct serial* self)
{
    (void)self;
    return flush_rx_ctx(&g_usart0_ctx);
}

const struct serial_ops atmegaxx0_1_usart0_ops = {
    .initialize                = usart0_initialize,
    .transmit                  = usart0_transmit,
    .receive                   = usart0_receive,
    .set_rx_callback           = usart0_set_rx_callback,
    .enable_rx_interrupt       = usart0_enable_rx_interrupt,
    .set_rx_interrupt_priority = usart0_set_rx_interrupt_priority,
    .flush_tx                  = usart0_flush_tx,
    .flush_rx                  = usart0_flush_rx,
};

/* ========================================================================== */

/* USART1 op wrappers */

static int8_t usart1_initialize(struct serial* self)
{
    return initialize_ctx(&g_usart1_ctx, self);
}

static int8_t usart1_transmit(
    const struct serial* self, const uint8_t* buffer, size_t size)
{
    (void)self;
    return transmit_ctx(&g_usart1_ctx, buffer, size);
}

static int8_t usart1_receive(const struct serial* self, uint8_t* byte)
{
    (void)self;
    return receive_ctx(&g_usart1_ctx, byte);
}

static int8_t usart1_set_rx_callback(
    const struct serial* self,
    serial_rx_callback_t callback,
    void*                callback_context)
{
    (void)self;
    return set_rx_callback_ctx(&g_usart1_ctx, callback, callback_context);
}

static int8_t usart1_enable_rx_interrupt(const struct serial* self, bool enable)
{
    (void)self;
    return enable_rx_interrupt_ctx(&g_usart1_ctx, enable);
}

static int8_t usart1_set_rx_interrupt_priority(
    const struct serial* self, uint8_t priority)
{
    (void)self;
    return set_rx_interrupt_priority_ctx(&g_usart1_ctx, priority);
}

static int8_t usart1_flush_tx(const struct serial* self)
{
    (void)self;
    return flush_tx_ctx(&g_usart1_ctx);
}

static int8_t usart1_flush_rx(const struct serial* self)
{
    (void)self;
    return flush_rx_ctx(&g_usart1_ctx);
}

const struct serial_ops atmegaxx0_1_usart1_ops = {
    .initialize                = usart1_initialize,
    .transmit                  = usart1_transmit,
    .receive                   = usart1_receive,
    .set_rx_callback           = usart1_set_rx_callback,
    .enable_rx_interrupt       = usart1_enable_rx_interrupt,
    .set_rx_interrupt_priority = usart1_set_rx_interrupt_priority,
    .flush_tx                  = usart1_flush_tx,
    .flush_rx                  = usart1_flush_rx,
};

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__)

/* ========================================================================== */

/* USART2 op wrappers */

static int8_t usart2_initialize(struct serial* self)
{
    return initialize_ctx(&g_usart2_ctx, self);
}

static int8_t usart2_transmit(
    const struct serial* self, const uint8_t* buffer, size_t size)
{
    (void)self;
    return transmit_ctx(&g_usart2_ctx, buffer, size);
}

static int8_t usart2_receive(const struct serial* self, uint8_t* byte)
{
    (void)self;
    return receive_ctx(&g_usart2_ctx, byte);
}

static int8_t usart2_set_rx_callback(
    const struct serial* self,
    serial_rx_callback_t callback,
    void*                callback_context)
{
    (void)self;
    return set_rx_callback_ctx(&g_usart2_ctx, callback, callback_context);
}

static int8_t usart2_enable_rx_interrupt(const struct serial* self, bool enable)
{
    (void)self;
    return enable_rx_interrupt_ctx(&g_usart2_ctx, enable);
}

static int8_t usart2_set_rx_interrupt_priority(
    const struct serial* self, uint8_t priority)
{
    (void)self;
    return set_rx_interrupt_priority_ctx(&g_usart2_ctx, priority);
}

static int8_t usart2_flush_tx(const struct serial* self)
{
    (void)self;
    return flush_tx_ctx(&g_usart2_ctx);
}

static int8_t usart2_flush_rx(const struct serial* self)
{
    (void)self;
    return flush_rx_ctx(&g_usart2_ctx);
}

const struct serial_ops atmegaxx0_1_usart2_ops = {
    .initialize                = usart2_initialize,
    .transmit                  = usart2_transmit,
    .receive                   = usart2_receive,
    .set_rx_callback           = usart2_set_rx_callback,
    .enable_rx_interrupt       = usart2_enable_rx_interrupt,
    .set_rx_interrupt_priority = usart2_set_rx_interrupt_priority,
    .flush_tx                  = usart2_flush_tx,
    .flush_rx                  = usart2_flush_rx,
};

/* ========================================================================== */

/* USART3 op wrappers */

static int8_t usart3_initialize(struct serial* self)
{
    return initialize_ctx(&g_usart3_ctx, self);
}

static int8_t usart3_transmit(
    const struct serial* self, const uint8_t* buffer, size_t size)
{
    (void)self;
    return transmit_ctx(&g_usart3_ctx, buffer, size);
}

static int8_t usart3_receive(const struct serial* self, uint8_t* byte)
{
    (void)self;
    return receive_ctx(&g_usart3_ctx, byte);
}

static int8_t usart3_set_rx_callback(
    const struct serial* self,
    serial_rx_callback_t callback,
    void*                callback_context)
{
    (void)self;
    return set_rx_callback_ctx(&g_usart3_ctx, callback, callback_context);
}

static int8_t usart3_enable_rx_interrupt(const struct serial* self, bool enable)
{
    (void)self;
    return enable_rx_interrupt_ctx(&g_usart3_ctx, enable);
}

static int8_t usart3_set_rx_interrupt_priority(
    const struct serial* self, uint8_t priority)
{
    (void)self;
    return set_rx_interrupt_priority_ctx(&g_usart3_ctx, priority);
}

static int8_t usart3_flush_tx(const struct serial* self)
{
    (void)self;
    return flush_tx_ctx(&g_usart3_ctx);
}

static int8_t usart3_flush_rx(const struct serial* self)
{
    (void)self;
    return flush_rx_ctx(&g_usart3_ctx);
}

const struct serial_ops atmegaxx0_1_usart3_ops = {
    .initialize                = usart3_initialize,
    .transmit                  = usart3_transmit,
    .receive                   = usart3_receive,
    .set_rx_callback           = usart3_set_rx_callback,
    .enable_rx_interrupt       = usart3_enable_rx_interrupt,
    .set_rx_interrupt_priority = usart3_set_rx_interrupt_priority,
    .flush_tx                  = usart3_flush_tx,
    .flush_rx                  = usart3_flush_rx,
};

#endif /* 100-pin variants */

/* ========================================================================== */

/* Interrupt vectors. avr-libc declares the RX-complete vector for each
 * present USART; defining the ISR here binds it to our per-instance
 * dispatch. */

ISR(USART0_RX_vect)
{
    handle_rx_isr(&g_usart0_ctx);
}

ISR(USART1_RX_vect)
{
    handle_rx_isr(&g_usart1_ctx);
}

#if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__)
ISR(USART2_RX_vect)
{
    handle_rx_isr(&g_usart2_ctx);
}

ISR(USART3_RX_vect)
{
    handle_rx_isr(&g_usart3_ctx);
}
#endif

/* ========================================================================== */
