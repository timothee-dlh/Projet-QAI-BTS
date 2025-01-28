#ifndef ADC_READ_H
#define ADC_READ_H

void adc_read(void *arg);

typedef struct timer_callback_args {
        int raw;
        adc_unit_t *unit_39;
        adc_channel_t *channel_39;
        adc_oneshot_unit_handle_t unit_39_handle;
}timer_callback_args;

#endif
