#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>

int main() {
    struct mosquitto *mosq;
    int rc;

    mosquitto_lib_init();
    mosq = mosquitto_new("producer", true, NULL);

    rc = mosquitto_connect(mosq, "localhost", 1883, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error al conectar: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    printf("Conectado al broker MQTT.\n");

    const char *datos[] = {
        "{\"MATRICULA\": 226020039, \"NOMBRE\": \"Alejandro\", \"Pap\": \"Miranda\", \"Sap\": \"Martinez\", \"MATERIA\": \"Sistemas Operativos\", \"CALIFICACION\": \"95\", \"CARRERA\": \"Ingenieria en Sistemas\"}",
        "{\"MATRICULA\": 226020042, \"NOMBRE\": \"Anel Athziri\", \"Pap\":\"Cabañas\",\"Sap\":\"Santamaria\",\"MATERIA\":\"Sistemas Operativos\",\"CALIFICACION\":\"80\",\"CARRERA\":\"Ingenieria en Sistemas\"}"
    };

    for (int i = 0; i < 2; i++) {
        printf("Publicando datos: %s\n", datos[i]);
        rc = mosquitto_publish(mosq, NULL, "datos/informacion", strlen(datos[i]), datos[i], 0, false);
        if (rc != MOSQ_ERR_SUCCESS) {
            fprintf(stderr, "Error al publicar: %s\n", mosquitto_strerror(rc));
            return 1;
        }
        printf("Datos publicados en el tópico 'datos/informacion'.\n");
    }

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    printf("Desconectado del broker MQTT.\n");

    return 0;
}
