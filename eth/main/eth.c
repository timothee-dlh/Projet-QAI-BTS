#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_eth.h"
#include "lwip/inet.h"  
#include "esp_eth_mac_esp.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

static const char *TAG = "ETH";
static int CONNECTED = 0;    
esp_eth_handle_t eth_handle = NULL;

static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        CONNECTED = 1;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        CONNECTED = 0;
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


void app_main(void)
{
    gpio_reset_pin(16); // Reset en pin of the oscillator
    gpio_set_direction(16, GPIO_MODE_OUTPUT); // Set pin as output

    gpio_set_level(16, 1); // Set pin high, Enable 50MHz crystal 
                           
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init()); // Initialize Lwip TCP/IP Stack
    ESP_LOGI("NETIF: ", "Netif initialized");
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create event loop for event handlers

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH(); // Netif config
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg); // Create a new TCP/IP stack
    if(eth_netif != NULL){
        ESP_LOGI("NETIF:", "Netif new created");
    } else {
        ESP_LOGE("NETIF:", "Error while creating Netif");
    }


    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
        emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
        emac_config.clock_config.rmii.clock_gpio = EMAC_CLK_IN_GPIO;

    ESP_LOGI("Event Handler :", "Event handler registered");

    eth_mac_config_t mac_conf = ETH_MAC_DEFAULT_CONFIG();
    mac_conf.sw_reset_timeout_ms = 1000;

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_conf);
    ESP_LOGI("MAC :", "esp_eth_mac_new_esp32 done");
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = -1;

    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);
    if (phy != NULL){
        ESP_LOGI("PHY :", "esp_eth_phy_new_lan87xx done");
    } else {
        ESP_LOGE("PHY :", "esp_mac_phy_error");
    }
    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);

    eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle)); // Install the Ethernet Driver
    ESP_LOGI("Driver :", "esp_eth_driver_install done");

    esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)); // attach Ethernet driver to TCP/IP stack
                                                                    
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL)); // register user defined IP event handlers

    ESP_ERROR_CHECK(esp_eth_start(eth_handle)); // Start Ethernet interface

    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(eth_netif)); // Stop DHCP service to use Static IP conf

    esp_netif_ip_info_t ip_info;
    ip_info.ip.addr    = inet_addr("192.168.1.2"); // IP Configuration of the esp32
    ip_info.gw.addr    = inet_addr("192.168.1.1");
    ip_info.netmask.addr = inet_addr("255.255.255.0");
    ESP_ERROR_CHECK(esp_netif_set_ip_info(eth_netif, &ip_info));
    
    if(esp_netif_is_netif_up(eth_netif)){
        ESP_LOGI("NETIF :", "Interface is up");
    } else {
        ESP_LOGI("NETIF :", "Interface is down");
    }

    ESP_LOGI(TAG, "Static IP set: 192.168.1.2");

    /*
     *MQTT Client configuration
     */
    esp_mqtt_client_config_t mqtt_cfg = { 
        .broker.address.hostname = "192.168.1.1",
        .broker.address.port = 9001,
        .broker.address.transport = MQTT_TRANSPORT_OVER_WS,
        .session.keepalive = 120, // Increase keepalive interval
        .credentials.username = NULL,
        .credentials.authentication.password = NULL,
    };
    esp_mqtt_client_handle_t mqtt_handle = esp_mqtt_client_init(&mqtt_cfg); // Create a handle to mqtt client

    if(mqtt_handle == NULL){
        ESP_LOGE("MQTT", "Error when init mqtt");
    } else {
        ESP_LOGI("MQTT :", "mqtt init done");
    }
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_handle); // Register mqtt event handler
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_handle)); // Start the mqtt client
    ESP_LOGI("MQTT :", "mqtt started");

    while(1){
        if(CONNECTED == 1){
            esp_mqtt_client_publish(mqtt_handle, "test", "1234 from esp32", 0, 0, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            ESP_LOGI("MQTT :", "MQTT not connected");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

