/**
 * Absrtact DC Drive Project. Master Part
 *
 * make PROFILE=release LOG=1 tidy all
 * make PROFILE=debug LOG=1 tidy all
 */

#include "common_defs.h"
#include "can.h"

#include <abstractSTM32.h>
#include <abstractADC.h>
#include <abstractLOG.h>
#include <abstractCAN.h>
#include <abstractLCD.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/can.h>

struct abst_pin pot_ch = {
    .port = ABST_GPIOC,
    .num = 3,
    .mode = ABST_MODE_ANALOG,
    .adc_num = 1,
    .adc_channel = 13,
    .adc_sample_time = ABST_ADC_SMPR_SMP_480CYC,
    .adc_resolution = ABST_ADC_RES_10BIT,
    .otype = ABST_OTYPE_PP,
    .speed = ABST_OSPEED_50MHZ,
    .pull_up_down = ABST_PUPD_NONE,
    .is_inverse = false
};

volatile int32_t des_speed = 0;
const uint16_t time_delta = 100;

void log_usart(void);
void set_pid_coef(enum pid_types type, enum pid_fields field, int32_t value);
void save_pid_to_flash(enum pid_types type);
void set_desired_value(enum pid_types type, int32_t value);
void request_log(uint8_t log_type);

int main(void)
{
    rcc_set_ppre1(RCC_CFGR_PPRE_DIV_16);
    rcc_apb1_frequency = 16e6 / 16;
    abst_init(16e6, 700);
    abst_log_init(9600);
    
    abst_logf("Inited\n");
    can_bus_init();
    abst_logf("CAN inited\n");
    
    struct abst_pin *pins_arr[] = {&pot_ch};
    uint8_t N = sizeof(pins_arr) / sizeof(pins_arr[0]);
    volatile uint16_t adc_vals[N];

    volatile uint16_t *input = adc_vals;

    enum abst_errors err = abst_adc_read_cont(pins_arr, // Array of pins
                                              adc_vals, // Array of values
                                              N, // Length of array
                                              8,  // Prescale
                                              1); // Prior of DMA requests

    set_pid_coef(SPEED_PID, PID_FIELD_P, 1e6);
    set_pid_coef(SPEED_PID, PID_FIELD_I, 1e3);
    set_pid_coef(SPEED_PID, PID_FIELD_D, 0);
    set_pid_coef(SPEED_PID, PID_FIELD_DIV, 1e6);
//     save_pid_to_flash(SPEED_PID);
    
    abst_delay_ms(10);
    while (1) {
        des_speed = *input - 512;
        set_desired_value(SPEED_PID, des_speed);
        
        request_log(LOG_SPEED);
        
        log_usart();
        abst_delay_ms(50);
    }
}

void log_usart(void)
{
    static log = 0;
    if (log % 1 == 0) {
        log = 0;
        
        abst_logf("%i, %i\n", (int)des_speed, (int)log_get_speed());
    }
    log++;
}

void set_pid_coef(enum pid_types type, enum pid_fields field, int32_t value)
{
    struct pid_settings_msg set_msg = {
        .pid_type = type,
        .pid_field = field,
        .value = value,
    };
    
    can_transmit(   CAN1,
                    200,     /* (EX/ST)ID: CAN ID */
                    false, /* IDE: CAN ID extended? */
                    false, /* RTR: Request transmit? */
                    sizeof(set_msg),     /* DLC: Data length */
                    &set_msg);
    
    abst_delay_ms(time_delta);
}

void save_pid_to_flash(enum pid_types type)
{
    const uint8_t type_byte = type;
    can_transmit(   CAN1,
                    200,     /* (EX/ST)ID: CAN ID */
                    false, /* IDE: CAN ID extended? */
                    false, /* RTR: Request transmit? */
                    1,     /* DLC: Data length */
                    &type_byte);
    
    abst_delay_ms(time_delta);
}

void set_desired_value(enum pid_types type, int32_t value)
{
    struct pid_des_value_msg des_msg = {
        .pid_type = SPEED_PID,
        .value = value,
    };
    
    can_transmit(   CAN1,
                    100,     /* (EX/ST)ID: CAN ID */
                    false, /* IDE: CAN ID extended? */
                    false, /* RTR: Request transmit? */
                    sizeof(des_msg),     /* DLC: Data length */
                    &des_msg);
    
    abst_delay_ms(time_delta);
}

void request_log(uint8_t log_type)
{
    can_transmit(   CAN1,
                    800,        /* (EX/ST)ID: CAN ID */
                    false,      /* IDE: CAN ID extended? */
                    false,      /* RTR: Request transmit? */
                    sizeof(log_type),     /* DLC: Data length */
                    &log_type);
//     abst_logf("Requested log: %i\n", log_type);
    abst_delay_ms(time_delta);
}
