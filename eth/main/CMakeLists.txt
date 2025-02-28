idf_component_register(SRCS "eth.c"
                    REQUIRES esp_eth esp_netif nvs_flash esp_driver_gpio mqtt esp-tls
                    INCLUDE_DIRS ".")
