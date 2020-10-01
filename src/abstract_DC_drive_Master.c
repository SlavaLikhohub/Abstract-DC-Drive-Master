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

#include <string.h>

struct abst_lcd lcd = {
    .port = ABST_GPIOE,
    .VO = -1,
    .RC = 7,
    .RW = 10,
    .E = 11,
    .DB = 1 << 12 | 1 << 13 | 1 << 14 | 1 << 15,
    .LED = 9,
    .pwm_setting = ABST_SOFT_PWM,
    .delay_ms = 0,
    .delay_us = 0,
    .is_half_byte = 1
};

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

uint16_t buttons_inf = 0;
enum buttons_dir {
    BUTTON_UP = 1 << 0,
    BUTTON_DOWN = 1 << 1,
    BUTTON_LEFT = 1 << 2,
    BUTTON_RIGHT = 1 << 3,
};
struct abst_pin_group buttons = {
    .port = ABST_GPIOC,
    .num = 1 << 6 | 1 << 8 | 1 << 9 | 1 << 11,
    .mode = ABST_MODE_INPUT,
    .otype = ABST_OTYPE_PP,
    .speed = ABST_OSPEED_2MHZ,
    .pull_up_down = ABST_PUPD_NONE,
    .is_inverse = 0b1111
};

volatile enum pid_types pid_mode = NONE;
volatile int32_t des_speed = 0;
volatile int32_t des_current = 0;
volatile int32_t des_position = 0;
volatile int32_t des_voltage = 0;

void log_usart(void);

int main(void)
{
    rcc_set_ppre1(RCC_CFGR_PPRE_DIV_16);
    rcc_apb1_frequency = 16e6 / 16;
    abst_init(16e6, 0);
    abst_log_init(19200);
    abst_lcd_init(&lcd);
    abst_lcd_set_led(&lcd, 0);
    
    abst_group_gpio_init(&buttons);
    
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
    
    set_pid_coef(CURRENT_PID, PID_FIELD_P, 3e5);
    set_pid_coef(CURRENT_PID, PID_FIELD_I, 100);
    set_pid_coef(CURRENT_PID, PID_FIELD_D, 0);
    set_pid_coef(CURRENT_PID, PID_FIELD_AVER_N, 50);
    set_pid_coef(CURRENT_PID, PID_FIELD_DIV, -1e6);
    
    set_pid_coef(POSITION_PID, PID_FIELD_P, 8e5);
    set_pid_coef(POSITION_PID, PID_FIELD_I, 0);
    set_pid_coef(POSITION_PID, PID_FIELD_D, 0);
    set_pid_coef(POSITION_PID, PID_FIELD_DIV, 1e6);
//     save_pid_to_flash(SPEED_PID);
    
    abst_delay_ms(10);
    while (1) {
        des_speed = (*input - 512) / 50 * 50;
        des_current = *input / 20 * 20;
        des_position = (*input * 10) / 500 * 500;
        des_voltage = (*input - 512) / 2;
        
        buttons_inf = abst_group_digital_read(&buttons);
        switch (buttons_inf) {
            case BUTTON_UP:
                pid_mode = SPEED_PID;
                break;
            case BUTTON_RIGHT:
                pid_mode = CURRENT_PID;
                break;
            case BUTTON_DOWN:
                pid_mode = POSITION_PID;
                break;
            case BUTTON_LEFT:
                pid_mode = DIRECT_CONT;
                break;
        }
        
        int32_t msg = 0;
        enum log_types log_type = 0;
        switch (pid_mode) {
            case SPEED_PID:
                msg = des_speed;
                log_type = LOG_SPEED;
                break;
            case CURRENT_PID:
                msg = des_current;
                log_type = LOG_CURRENT;
                break;
            case POSITION_PID:
                msg = des_position;
                log_type = LOG_POSITION;
                break;
            case DIRECT_CONT:
                msg = des_voltage;
                log_type = LOG_VOLTAGE;
                break;
            default:
                msg = 0;
        }
        set_desired_value(pid_mode, msg);
        
        request_log(log_type);
        
        log_usart();
        abst_delay_ms(50);
    }
}

void log_usart(void)
{
    
    char name[10];
    int32_t des_val = 0;
    int32_t log_val = 0;
    switch (pid_mode) {
        case SPEED_PID:
            strcpy(name, "Speed");
            des_val = des_speed;
            log_val = log_get_speed();
            break;
        case CURRENT_PID:
            strcpy(name, "Current");
            des_val = des_current;
            log_val = log_get_current();
            break;
        case POSITION_PID:
            strcpy(name, "Pos");
            des_val = des_position;
            log_val = log_get_position();
            break;
        case DIRECT_CONT:
            strcpy(name, "Direct");
            des_val = des_voltage;
            log_val = log_set_motor_v();
            break;
        default:
            strcpy(name, "None");
    }
    abst_logf("%i, %i\n", (int)des_val, (int)log_val);
    
    static int16_t log = 0;
    
    if (log % 5 == 0) {
        log = 0;
        
        abst_lcd_clear_disp(&lcd);
        abst_lcd_put_str_f(&lcd, "%s: %i", name, (int)des_val);
        abst_lcd_set_cursor(&lcd, 0, 11);
        abst_lcd_put_str_f(&lcd, "M:%i", (int)pid_mode);
        abst_lcd_set_cursor(&lcd, 1, 0);
        abst_lcd_put_str_f(&lcd, "FeedBack: %i", (int)log_val);
    }
    log++;
}
