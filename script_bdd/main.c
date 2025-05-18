#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mosquitto.h>
#include <mysql.h>

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message);
void cleanup(MYSQL *conn, struct mosquitto *mqtt_handle);

MYSQL *conn;

char * arg;

int main(int argc, char **argv){
    if(argc < 8){ // Usage
        printf("Usage: ./mqtt_to_bdd.elf hôte port topic utilisateur_sql mdp_bdd base_sql nom_capteur\n");
        return 1;
    }
  arg = argv[7];

    conn = mysql_init(NULL); // Initialise l'instance mariadb

    if (conn == NULL) {
        fprintf(stderr, "Erreur d'initialisation de MySQL: %s\n", mysql_error(conn));
        exit(1);
    } else printf("SQL instance initiated\n");

    if (mysql_real_connect(conn, "127.0.0.1", argv[4] , argv[5], argv[6], 3306, NULL, 0) == NULL) { // Connecte à la base de données
        fprintf(stderr, "Erreur de connexion à la base de données: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }else printf("Connection to the database succesful\n");

    if(mosquitto_lib_init() != MOSQ_ERR_SUCCESS){ // Alloue les ressources requises par la libraire MQTT
        printf("Error while init lib\n");
        exit(1);
    } 

    void *user_data = NULL;
    struct mosquitto *mqtt_handle = mosquitto_new(argv[3], 0, user_data); // Crée une nouvelle instance MQTT
    if(mqtt_handle == NULL){
        printf("Error while creating new instance of mosquitto\n");
        cleanup(conn, mqtt_handle);
        exit(1);
    } 
   
    mosquitto_message_callback_set(mqtt_handle, mqtt_callback);
    
    mosquitto_username_pw_set(mqtt_handle, "esp", "esp");
    
  if(mosquitto_connect(mqtt_handle, argv[1], atoi(argv[2]), 10) != MOSQ_ERR_SUCCESS){ // Connecte au broker MQTT
        printf("Error while connecting to mqtt broker\n");
        cleanup(conn, mqtt_handle);
        exit(1);
    } else printf("Connection to the broker successful\n");

    int err = 0;
    while((err = mosquitto_subscribe(mqtt_handle, NULL, argv[3], 1) != MOSQ_ERR_SUCCESS)){ // Souscrit au topic passé en argument du programme
        printf("Error while subscribing :%d\nRetrying...", err);
        sleep(1);
    }
    printf("Subscribing to topic %s succesful\n", argv[3]);

    err = mosquitto_loop_forever(mqtt_handle, -1, 1);
    if(err != MOSQ_ERR_SUCCESS){
        printf("Error while creating the mosquitto loop : %d\n", err);
        cleanup(conn, mqtt_handle);
        exit(1);
    }
    cleanup(conn, mqtt_handle);
    return 0;
}

void mqtt_callback(struct mosquitto *mqtt, void *user_data, const struct mosquitto_message *message){
     if (message->payloadlen) {
        char query[512];
        // Insertion des données dans la base de données
        snprintf(query, 256, "INSERT INTO %s (valeur, nom_capteur) VALUES ('%s', '%s')", arg, (char *)message->payload, (char *)message->topic);
        // Log de la Query dans le terminal
        printf("Query: %s\n", query);
        if (mysql_query(conn, query)) {
            fprintf(stderr, "Erreur lors de l'exécution de la requête: %s\n", mysql_error(conn));
        }
    } else {
        // Si le message est vide alors log dans le terminal
        printf("Message reçu %s (message vide)\n", message->topic);
    }
}

void cleanup(MYSQL *conn, struct mosquitto *mqtt_handle){

    mysql_close(conn);
    mosquitto_destroy(mqtt_handle); // Supprime l'instance MQTT
    mosquitto_lib_cleanup(); // Libère les ressources
}
