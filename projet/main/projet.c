#include "hal/adc_types.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "adc_read.h"
#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "esp_eth_mac_esp.h"
#include "esp_eth_phy.h"

void app_main(void)
{
        static const char* TAG = "main"; // Tag pour les LOG

        adc_unit_t unit_39;
        adc_channel_t channel_39;
        ESP_ERROR_CHECK(adc_oneshot_io_to_channel(39, &unit_39, &channel_39 )); // Retourne l'unité (ADC1-2) et le channel (Channel0-9) adc pour le pin 39

        /*
         * Configuration de l'unité ADC correspondant au pin 39
         */
        adc_oneshot_unit_init_cfg_t config_adc_39 = {
                .unit_id = unit_39,
                .ulp_mode = ADC_ULP_MODE_DISABLE
        };

        /*
         * Configure le channel correspondant au pin 39
         */
        adc_oneshot_chan_cfg_t channel_config_39 = {
                .atten = ADC_ATTEN_DB_0,
                .bitwidth = ADC_BITWIDTH_DEFAULT
        };

        /*
         * Crée le handle pour manipuler l'ADC sur le pin 39
         */
        adc_oneshot_unit_handle_t unit_39_handle;

        ESP_ERROR_CHECK(adc_oneshot_new_unit(&config_adc_39, &unit_39_handle)); // Enregistre la nouvelle unité d'ADC
        ESP_ERROR_CHECK(adc_oneshot_config_channel(unit_39_handle, channel_39, &channel_config_39)); // Configure cette unité
        ESP_LOGI(TAG, "Unit: %d Channel: %d", unit_39, channel_39); // Log la configuration sur le port console

        timer_callback_args timer_call_conf = {
                .raw = 0,
                .unit_39 = &unit_39,
                .channel_39 = &channel_39,
                .unit_39_handle = unit_39_handle
        };
        
        esp_timer_create_args_t timer_args = {
               .callback = &adc_read,
               .arg = &timer_call_conf,
               .name = "adc_timer"
        };
        esp_timer_handle_t timer_handle;

        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle));
        ESP_LOGI(TAG, "After timer_create"); 
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 1000));
        ESP_LOGI(TAG, "After timer_start"); 

        while(1){
                adc_read(&timer_args); // Fais quoi ??? Lis les valeurs en continu ou attends l'interrupt ?
        }

        ESP_ERROR_CHECK(esp_timer_deinit());
}

