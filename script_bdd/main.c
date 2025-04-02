#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    struct mosquitto *mqtt_handle = mosquitto_new("Script MQTT_BDD", 0, user_data); // New instance of mosquitto
    if(mqtt_handle == NULL){
        printf("Error while creating new instance of mosquitto\n");
        mosquitto_destroy(mqtt_handle); // Remove mqtt instance 
        mosquitto_lib_cleanup(); // Cleanup ressources
        return 1;
    } 
   
    mosquitto_message_callback_set(mqtt_handle, mqtt_callback);

    if(mosquitto_connect(mqtt_handle, argv[1], atoi(argv[2]), 10) != MOSQ_ERR_SUCCESS){ // Connect to mqtt broker
        printf("Error while connecting to mqtt broker\n");
        return 1;
    } else printf("Connection to the broker successful\n");

    int err = 0;
    while((err = mosquitto_subscribe(mqtt_handle, NULL, argv[3], 1) != MOSQ_ERR_SUCCESS)){ // Subscribe to topic
        printf("Error while subscribing :%d\nRetrying...", err);
        sleep(1);
    }
    printf("Subscribing to topic %s succesful\n", argv[3]);

    //mosquitto_loop_forever(mqtt_handle, -1, 1);
    err = mosquitto_loop_start(mqtt_handle);
    if(err != MOSQ_ERR_SUCCESS){
        printf("Error while creating thread : %d\n", err);
        return 1;
    }else {
        printf("Thread spawned succesfully\n");
    }
    while(1){sleep(1);} // Use that for mariadb post or callback ???

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
