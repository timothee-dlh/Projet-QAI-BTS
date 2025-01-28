#include "hal/adc_types.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "adc_read.h"
#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "esp_eth_mac_esp.h"
#include "esp_eth_phy.h"

// ETH PHY CHIP: esp_eth_phy_new_lan87xx
void app_main(void)
{
        static const char* TAG = "main";

        adc_unit_t unit_39;
        adc_channel_t channel_39;
        ESP_ERROR_CHECK(adc_oneshot_io_to_channel(39, &unit_39, &channel_39 ));

        adc_oneshot_unit_init_cfg_t config_adc_39 = {
                .unit_id = unit_39,
                .ulp_mode = ADC_ULP_MODE_DISABLE
        };

        adc_oneshot_chan_cfg_t channel_config_39 = {
                .atten = ADC_ATTEN_DB_0,
                .bitwidth = ADC_BITWIDTH_DEFAULT
        };

        adc_oneshot_unit_handle_t unit_39_handle;

        ESP_ERROR_CHECK(adc_oneshot_new_unit(&config_adc_39, &unit_39_handle));
        ESP_ERROR_CHECK(adc_oneshot_config_channel(unit_39_handle, channel_39, &channel_config_39));
        ESP_LOGI(TAG, "Unit: %d Channel: %d", unit_39, channel_39); 

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
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, 1000));

        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();            
        eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
        esp32_emac_config.smi_gpio.mdc_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;          
        esp32_emac_config.smi_gpio.mdio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;         
        esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config); 

        eth_phy_config_t phy_conf = {
                .phy_addr = -1,
                .reset_timeout_ms = 100,
                .autonego_timeout_ms = 5000,
                .reset_gpio_num = -1,
                .hw_reset_assert_time_us = 0,
                .post_hw_reset_delay_ms = 0
        };
        esp_eth_phy_new_lan87xx(&phy_conf);
        while(1){

        }

        ESP_ERROR_CHECK(esp_timer_deinit());
}

