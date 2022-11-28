
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h> // Requiere link con lpthread

#ifndef ADDRESS
#define ADDRESS "127.0.0.1"
#endif

#ifndef PORT
#define PORT 8000
#endif

void *connection_handler(void*);

int finished_thread = 0;

int main(int argc, char** argv) {
  if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    printf("BIENVENIDO AL CLIENTE DEL SISTEMA DE LA UNIVERSIDAD NACIONAL DE LA MATANZA\n");
    printf("--------------------------------------\n");
    printf("SISTEMA DE ASISTENCIAS UNLAM\n");
    printf("TP3 - Ejercicio 5 - Grupo 4\n");
    printf("--------------------------------------\n");
    printf("INTEGRANTES\n");
    printf("Di Vito, Tomas\tDNI:39340228\n");
    printf("Estevez, Adrian\t39325872\n");
    printf("Fernandez, Matias\tDNI:38613699\n");
    printf("Parra, Martin\tDNI:40012233\n");
    printf("Mirò, Agustin\tDNI:40923621\n");
    printf("--------------------------------------\n");
		printf("El cliente se va a conectar al servidor corriendo en 127.0.0.1:8000\n");
		printf("Una vez conectado se le pedira al usuario su nombre de usuario y contrasenia,\n");
		printf("Una vez logueado como docente podra consultar una fecha con formato yyyy-mm-dd\n");
		printf("y en caso de no existir un archivo de asistencias se le iniciara el proceso de carga automatica\n");
		printf("y una vez con un archivo de asistencia, al consultar la fecha, se le mostrara la asistencia de ese dia.\n");
		printf("Si ingresa como alumno, el alumno podra consultar fechas y sera notificado si en esa fecha estuvo presente,\n");
		printf("ausente, o no existe registro para ese dia. Tambien, puede consultar mediante el mensaje 'ASISTENCIA'\n");
		printf("el porcentaje de asistencia que tiene el alumno, en base al total de clases registradas hasta el momento.\n");
    printf("--------------------------------------\n");
		printf("ADVERTENCIA!!!\n");
		printf("Si la conexion se pierde en el medio de la subida de asistencia, el archivo va a estar guardado\n");
		printf("con las asistencias que se pudieron cargar previo a la desconexion.\n");
    printf("--------------------------------------\n");
    fflush(stdout);
    return 0;
  }

	int                ret = 0;
	int                conn_fd;
	struct sockaddr_in server_addr = { 0 };
  char*  message                 = (char*)(malloc(2048));
  pthread_t hilo_escucha;


	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	ret = inet_pton(AF_INET, ADDRESS, &server_addr.sin_addr);

	if (ret != 1) {
		if (ret == -1) {
			perror("inet_pton");
		}
		return -1;
	}

	conn_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (conn_fd == -1) {
		perror("socket");
		return -1;
	}

	ret =
	  connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret == -1) {
		perror("connect");
		return -1;
	}

  pthread_create(&hilo_escucha, NULL, connection_handler, (void*)&conn_fd);

  
  while(finished_thread == 0) {
    fgets(message, 2048, stdin);
    fflush(stdin);

		if (strlen(message) == 1) {
			sprintf(message, ".");
		}

    //Send some data
    if (send(conn_fd, message, strlen(message) + 1, 0) < 0) {
        puts("Send failed");
        return 1;
    }

		memset(message, 0, 2048);
  }

  pthread_join(hilo_escucha, NULL);
	free(message);
	ret = shutdown(conn_fd, SHUT_RDWR);
	if (ret == -1) {
		perror("El servidor se encuentra apagado");
		return -1;
	}

	// Once the connection got properly terminated, now we can proceed with
	// actually closing the file descriptor
	ret = close(conn_fd);
	if (ret == -1) {
		perror("close");
		return -1;
	}

	return 0;
}

void *connection_handler(void *socket_desc) {
  int conn_fd = *(int*)socket_desc;
  char*  server_message = (char*)(malloc(2048));
  int                read_size;

  while((read_size = recv(conn_fd, server_message, 1025, 0)) > 0) {
    server_message[read_size] = '\0';
    fprintf(stdout, "%s", server_message);
		fflush(stdout);
		memset(server_message, 0, 2048);
  }

	finished_thread = 1;

	free(server_message);
}