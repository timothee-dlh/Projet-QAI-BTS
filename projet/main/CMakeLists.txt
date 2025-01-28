idf_component_register(SRCS "projet.c" "adc_read.c"
                    INCLUDE_DIRS "." "include" REQUIRES esp_adc esp_timer esp_eth)
