#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mariadb/mysql.h>
#include <mosquitto.h>
#include <json-c/json.h>

void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    if (rc == 0) {
        printf("Conectado al broker MQTT.\n");
        mosquitto_subscribe(mosq, NULL, "datos/informacion", 0);
    } else {
        fprintf(stderr, "Error al conectar: %s\n", mosquitto_strerror(rc));
    }
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    printf("Mensaje recibido en el tópico 'datos/informacion'. Payload: %s\n", (char *)message->payload);
    struct json_object *parsed_json;
    struct json_object *matricula, *nombre, *pap, *sap, *materia, *calificacion, *carrera;

    parsed_json = json_tokener_parse(message->payload);

    json_object_object_get_ex(parsed_json, "MATRICULA", &matricula);
    json_object_object_get_ex(parsed_json, "NOMBRE", &nombre);
    json_object_object_get_ex(parsed_json, "Pap", &pap);
    json_object_object_get_ex(parsed_json, "Sap", &sap);
    json_object_object_get_ex(parsed_json, "MATERIA", &materia);
    json_object_object_get_ex(parsed_json, "CALIFICACION", &calificacion);
    json_object_object_get_ex(parsed_json, "CARRERA", &carrera);

    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char *server = "localhost";
    char *user = "isc";
    char *password = "tesoem";
    char *database = "datos";

    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "Error al conectar: %s\n", mysql_error(conn));
        return;
    }
    printf("Conectado a la base de datos MySQL.\n");

    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO informacion (MATRICULA, NOMBRE, Pap, Sap, MATERIA, CALIFICACION, CARRERA) VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s')",
             json_object_get_int(matricula), json_object_get_string(nombre), json_object_get_string(pap), json_object_get_string(sap), json_object_get_string(materia), json_object_get_string(calificacion), json_object_get_string(carrera));

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al insertar datos: %s\n", mysql_error(conn));
    } else {
        printf("Datos insertados correctamente en la base de datos.\n");
    }

    mysql_close(conn);
}

int main() {
    struct mosquitto *mosq;

    mosquitto_lib_init();
    mosq = mosquitto_new("consumer", true, NULL);

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    int rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al conectar: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
