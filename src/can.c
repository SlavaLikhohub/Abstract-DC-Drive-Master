#include "can.h"
#include "usart_log.h"

#include <abstractCAN.h>
#include <abstractLOG.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/can.h>
#include <stddef.h>

struct abst_pin can_RX = {
    .port = ABST_GPIOD,
    .num = 0,
    .mode = ABST_MODE_AF,
    .af = 9,
    .otype = ABST_OTYPE_PP,
    .speed = ABST_OSPEED_50MHZ,
    .pull_up_down = ABST_PUPD_NONE,
    .is_inverse = false
};

struct abst_pin can_TX = {
    .port = ABST_GPIOD,
    .num = 1,
    .af = 9,
    .mode = ABST_MODE_AF,
    .af_dir = ABST_AF_OUTPUT,
    .otype = ABST_OTYPE_PP,
    .speed = ABST_OSPEED_50MHZ,
    .pull_up_down = ABST_PUPD_NONE,
    .is_inverse = false
};

uint16_t id_offset = 0;

static const struct abst_can can_settings =
{
    .can_num = 1,  /* Number of CAN (1, 2). */
    .ttcm = false, /* Time triggered communication mode */
    .abom = true,  /* Automatic bus-off management */
    .awum = false, /* Automatic wakeup mode. */
    .nart = false, /* No automatic retransmission */
    .rflm = false, /* Receive FIFO locked mode. */
    .txfp = false, /* Transmit FIFO priority */
    .sjw = CAN_BTR_SJW_1TQ, /* Resynchronization time quanta jump width. */
    .ts1 = CAN_BTR_TS1_3TQ, /* Time segment 1 time quanta width. */
    .ts2 = CAN_BTR_TS2_4TQ, /* Time segment 2 time quanta width. */
    .brp = 100, //4,           /* Baud rate prescaler */
    .loopback = false,   /* Loopback mode */
    .silent = false      /* Silent mode */
};

static struct abst_can_filter_32_bit LOG_SPEED_INFO = 
{
    .filter_id = 0, /* Filter ID */
    .id1 = 1000,     /* First message ID to match. Increasing by **id_offset** while initializtion */
    .id2 = 1000,     /* Second message ID to match. Used for brodcast */
    .fifo = 0,      /* FIFO ID. */
    .enable = true  /* Enable Filter */
};

static struct abst_can_filter_32_bit LOG_POSITION_INFO = 
{
    .filter_id = 1, /* Filter ID */
    .id1 = 1100,     /* First message ID to match. Increasing by **id_offset** while initializtion */
    .id2 = 1100,     /* Second message ID to match. Used for brodcast */
    .fifo = 0,      /* FIFO ID. */
    .enable = true  /* Enable Filter */
};

static struct abst_can_filter_32_bit LOG_CURRENT_INFO = 
{
    .filter_id = 2, /* Filter ID */
    .id1 = 1200,     /* First message ID to match. Increasing by **id_offset** while initializtion */
    .id2 = 1200,     /* Second message ID to match. Used for brodcast */
    .fifo = 0,      /* FIFO ID. */
    .enable = true  /* Enable Filter */
};

static struct abst_can_filter_32_bit LOG_TEMPERATURE_INFO = 
{
    .filter_id = 3, /* Filter ID */
    .id1 = 1300,     /* First message ID to match. Increasing by **id_offset** while initializtion */
    .id2 = 1300,     /* Second message ID to match. Used for brodcast */
    .fifo = 0,      /* FIFO ID. */
    .enable = true  /* Enable Filter */
};

bool can_bus_init(void)
{  
    abst_gpio_init(&can_RX);
    abst_gpio_init(&can_TX);
    
    enum abst_errors init_status = abst_can_init(&can_settings);
    if (init_status != ABST_OK) {
        abst_log("Init CAN failed\n");
        return false;
    }
    LOG_SPEED_INFO.id1          += id_offset;
    LOG_POSITION_INFO.id1       += id_offset;
    LOG_CURRENT_INFO.id1        += id_offset;
    LOG_TEMPERATURE_INFO.id1    += id_offset;
    
//     abst_can_init_filter_32_bit(&LOG_SPEED_INFO);
//     abst_can_init_filter_32_bit(&LOG_POSITION_INFO);
//     abst_can_init_filter_32_bit(&LOG_CURRENT_INFO);
//     abst_can_init_filter_32_bit(&LOG_TEMPERATURE_INFO);
    
    /* CAN filter 0 init. */
    can_filter_id_mask_32bit_init(
                0,     /* Filter ID */
                0,     /* CAN ID */
                0,     /* CAN ID mask */
                0,     /* FIFO assignment (here: FIFO0) */
                true); /* Enable the filter. */
    
    /* Enable CAN interrupts. */
    cm_enable_interrupts();
#ifdef STM32F1
    nvic_enable_irq(NVIC_USB_HP_CAN_TX_IRQ);
    nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
#endif // STM32F1
#ifdef STM32F4
    nvic_enable_irq(NVIC_CAN1_TX_IRQ);
    nvic_enable_irq(NVIC_CAN1_RX0_IRQ);
#endif // STM32F4
    
    can_enable_irq(CAN1, CAN_IER_FMPIE0);
    can_enable_irq(CAN1, CAN_IER_FMPIE1);
    
    abst_log("Init CAN success\n");
    return true;
}

// Interrupt handler
void can1_rx0_isr(void)
{
//     abst_log("\nCAN interrupt\n");
//     abst_logf("Fifo pending: %i\n", abst_can_get_fifo_pending(1, 0));

    uint8_t fifo = 0;
    if (abst_can_get_fifo_pending(1, 0))
        fifo = 0;
    else if (abst_can_get_fifo_pending(1, 1)) 
        fifo = 1;
    
    uint32_t id = 0;
    bool ext = 0;
    bool rtr = 0;
    uint8_t fmi = 0;
    uint8_t length = 0;
    uint8_t data[64];
    
    can_receive(CAN1,
                fifo,   /* FIFO */
                true,   /* Release the FIFO automatically after coping data out. */
                &id,    /* Message ID */
                &ext,   /* The message ID is extended */
                &rtr,   /* Request of transmission */
                &fmi,   /* ID of the matched filter */
                &length,/* Length of message payload */
                data,   /* Message payload data */
                NULL);  /* Pointer to store the message timestamp */
    
//     abst_logf("CAN receive: Id: %i, fil: %i, len: %i\n", (int)id, (int)fmi, (int)length);
    
    if (id == LOG_SPEED_INFO.id1) {
        int32_t *speed = data;
        log_set_speed(*speed);
    }
    else if (id == LOG_POSITION_INFO.id1) {
        int32_t *position = data;
        log_set_position(*position);
    }
    else if (id == LOG_CURRENT_INFO.id1) {
        int32_t *current = data;
        log_set_current(*current);
    }
    else if (id == LOG_TEMPERATURE_INFO.id1) {
        int32_t *temp = data;
        log_set_temperature(*temp);
    }
}
