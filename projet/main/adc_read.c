#include <stdio.h>
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "adc_read.h"

void adc_read(void *arg){
        timer_callback_args *conf = (timer_callback_args *)arg; 
        ESP_ERROR_CHECK(adc_oneshot_read(conf->unit_39_handle, *conf->channel_39, &conf->raw));
        (void)printf("Raw Value: %d\n", conf->raw);
} 

