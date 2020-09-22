/**
 * Absrtact DC Drive Project. Master Part
 *
 * make PROFILE=release LOG=1 tidy all
 */

#include <abstractSTM32.h>
#include <abstractADC.h>
#include <abstractLOG.h>
#include <abstractCAN.h>

struct abst_pin pot_ch = {
    .port = ABST_GPIOA,
    .num = 0,
    .mode = ABST_MODE_ANALOG,
    .adc_num = 1,
    .adc_channel = 0,
    .adc_sample_time = ABST_ADC_SMPR_SMP_480CYC,
    .adc_resolution = ABST_ADC_RES_10BIT,
    .otype = ABST_OTYPE_OD,
    .speed = ABST_OSPEED_2MHZ,
    .pull_up_down = ABST_PUPD_NONE,
    .is_inverse = false
};

int main(void)
{
    abst_init(16e6, 700);
    abst_log_init(9600);

    struct abst_pin *pins_arr[] = {&pot_ch};
    uint8_t N = sizeof(pins_arr) / sizeof(pins_arr[0]);
    volatile uint16_t adc_vals[N];

    volatile uint16_t *input = adc_vals;
    volatile uint16_t *current = adc_vals + 1;

    enum abst_errors err = abst_adc_read_cont(pins_arr, // Array of pins
                                              adc_vals, // Array of values
                                              N, // Length of array
                                              8,  // Prescale
                                              1); // Prior of DMA requests

    uint32_t log = 0;
    while (1) {
        
        int16_t des_speed = *input / 2 - 255;
        
        if (log % 50 == 0) {
            log = 0;
//             abst_logf("Input: %i, Speed: %i\n", (int)des_speed, (int)fd_speed);
        }
        log++;
    }
}
