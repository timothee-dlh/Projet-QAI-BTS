#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mosquitto.h>
#include <mysql.h>

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message);
void cleanup(MYSQL *conn, struct mosquitto *mqtt_handle);

MYSQL *conn;

int main(int argc, char **argv){
    if(argc < 6){ // Usage
        printf("Usage: ./mqtt_to_bdd.elf host port topic sql_username sql_password\n");
        return 1;
    }

    conn = mysql_init(NULL); // Initialize the sql instance

    if (conn == NULL) {
        fprintf(stderr, "Erreur d'initialisation de MySQL: %s\n", mysql_error(conn));
        exit(1);
    } else printf("SQL instance initiated\n");

    if (mysql_real_connect(conn, "localhost", argv[4] , argv[5], "Projet", 0, NULL, 0) == NULL) { // Connect to database
        fprintf(stderr, "Erreur de connexion à la base de données: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }else printf("Connection to the database succesful\n");

    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS){ // Allocate ressources for mosquitto library
        printf("Error while init lib\n");
        exit(1);
    } 

    void *user_data = NULL;
    struct mosquitto *mqtt_handle = mosquitto_new("Script MQTT_BDD", 0, user_data); // New instance of mosquitto
    if(mqtt_handle == NULL){
        printf("Error while creating new instance of mosquitto\n");
        cleanup(conn, mqtt_handle);
        exit(1);
    } 
   
    mosquitto_message_callback_set(mqtt_handle, mqtt_callback);

    if(mosquitto_connect(mqtt_handle, argv[1], atoi(argv[2]), 10) != MOSQ_ERR_SUCCESS){ // Connect to mqtt broker
        printf("Error while connecting to mqtt broker\n");
        cleanup(conn, mqtt_handle);
        exit(1);
    } else printf("Connection to the broker successful\n");

    int err = 0;
    while((err = mosquitto_subscribe(mqtt_handle, NULL, argv[3], 1) != MOSQ_ERR_SUCCESS)){ // Subscribe to topic
        printf("Error while subscribing :%d\nRetrying...", err);
        sleep(1);
    }
    printf("Subscribing to topic %s succesful\n", argv[3]);

    err = mosquitto_loop_forever(mqtt_handle, -1, 1);
    if(err != MOSQ_ERR_SUCCESS){
        printf("Error while creating thread : %d\n", err);
        cleanup(conn, mqtt_handle);
        exit(1);
    }
    cleanup(conn, mqtt_handle);
    return 0;
}

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message){
     if (message->payloadlen) {
        printf("Message reçu %s: %s\n", message->topic, (char *)message->payload);
        char query[512];
        snprintf(query, 256, "INSERT INTO test (valeur, nom_capteur) VALUES ('%s', '%s')", (char *)message->payload, (char *)message->topic);
        printf("Query: %s\n", query);
        if (mysql_query(conn, query)) {
            fprintf(stderr, "Erreur lors de l'exécution de la requête: %s\n", mysql_error(conn));
        }
    } else {
        printf("Message reçu %s (message vide)\n", message->topic);
    }
}

void cleanup(MYSQL *conn, struct mosquitto *mqtt_handle){

    mysql_close(conn);
    mosquitto_destroy(mqtt_handle); // Remove mqtt instance 
    mosquitto_lib_cleanup(); // Cleanup ressources
}
