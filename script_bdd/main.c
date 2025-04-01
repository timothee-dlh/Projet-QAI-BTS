#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message);

int main(int argc, char **argv){
    if(argc < 4){ // Usage
        printf("Usage: ./mqtt_to_bdd.elf host port topic\n");
        return 1;
    }

    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS){ // Allocate ressources for mosquitto library
        printf("Error while init lib\n");
        return 1;
    } 

    void *user_data = NULL;
    struct mosquitto *mqtt_handle = mosquitto_new(NULL, 0, user_data); // New instance of mosquitto
    
    mosquitto_message_callback_set(mqtt_handle, mqtt_callback);

    if(mosquitto_connect(mqtt_handle, argv[1], atoi(argv[2]), 10) != MOSQ_ERR_SUCCESS){ // Connect to mqtt broker
        printf("Error while connecting to mqtt broker\n");
        return 1;
    } else printf("Connection to the broker successful\n");
    (void)mosquitto_subscribe(mqtt_handle, NULL, argv[3], 1); // Subscribe to topic
    //mosquitto_loop_start(mqtt_handle);
    mosquitto_loop_forever(mqtt_handle, -1, 1);

    mosquitto_destroy(mqtt_handle); // Remove mqtt instance 

    mosquitto_lib_cleanup(); // Cleanup ressources
}

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message){
     if (message->payloadlen) {
        printf("Message reçu %s: %s\n", message->topic, (char *)message->payload);
    } else {
        printf("Message reçu %s (message vide)\n", message->topic);
    }
}
