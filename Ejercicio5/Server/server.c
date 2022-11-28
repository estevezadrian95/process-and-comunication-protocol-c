#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h> // For listing files in the folder
#include <unistd.h>
#include <pthread.h> // Requiere link con lpthread
#include <signal.h>

#define PORT 8000

#define CLIENT_CONNECTED 0
#define CLIENT_MESSAGE 1
#define SERVER_MESSAGE 2
#define CLIENT_DISCONNECTION 3

struct User {
  char role;
  char name[30];
  int comision;
};

void *connection_handler(void*);
void cargar_archivo(FILE**, char*, char*);
void server_log(size_t, int, char*);
void login(char*, char*, struct User**);
int check_for_file(char*);
void SIGN_HANDLER(int);
void cargar_asistencia(char*, char*);
int asistio(char*, char*);
int calcular_porcentaje_asistencia(char*, int);
void traer_listado_alumnos_comision(int, char*[100], int*);
int fecha_valida(char*);

pthread_mutex_t log_lock;
// Cuando accedemos a archivos lockeamos
// en caso de que otro usuario pueda querer
// acceder a un archivo en el mismo momento.
// Hacemos que el que haya pedido algo con un archivo
// espere a que el otro finalice de leer/escribir a un archivo.
pthread_mutex_t acceso_a_archivo;
pthread_mutex_t asistencia_mutex;

FILE* log_file;
FILE* listado;

int server_socket;

int main(int argc, char** argv) {
  if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    printf("BIENVENIDO AL SERVIDOR DEL SISTEMA DE LA UNIVERSIDAD NACIONAL DE LA MATANZA\n");
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
    printf("El servidor permite la conexion de multiples clientes\n");
    printf("de manera asincrona con la utilizacion de pthread\n");
    printf("y sockets TCP.\n");
    printf("Se mantiene un log que se podra encontrar al correr el servidor\n");
    printf("en ./server.log\n");
    printf("Tambien se cargan las asistencias en la carpeta /Asistencia (que necesita estar presente para funcionar)\n");
    printf("El servidor se puede cerrar enviando una signal con Ctrl+C\n");
    printf("--------------------------------------\n");
    printf("COMO CORRERLO\n");
    printf("Simplemente poniendo ./server e iniciara en 127.0.0.1:8000\n");
    printf("--------------------------------------\n");
    fflush(stdout);
    return 0;
  }

  int max_clientes = 50;
  int clientes = 0;
  int contador = 0;
  pthread_t hilos_cliente[max_clientes];
  int client_socket = 0;
  server_socket = 0;

  cargar_archivo(&listado, "./listado", NULL);
  cargar_archivo(&log_file, "./server.log", "a+");

  signal(SIGINT, SIGN_HANDLER);

  struct sockaddr_in server_addr, client_addr;

  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Fallo el socket principal!");
    exit(EXIT_FAILURE);
  }

  // Seteando el tipo de socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bindeo del socket
  if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Error al bindear el puerto en el socket");
    exit(0);
  }

  listen(server_socket, 40);

  printf("SERVIDOR ARRANCADO EN PUERTO %d \n", PORT);
  fflush(stdout);

  socklen_t client_len = sizeof(client_addr);

  while ((client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len))) {
    if (pthread_create(&hilos_cliente[contador++], NULL, connection_handler, (void*)&client_socket) < 0) {
      perror("No se pudo crear el thread.");
      exit(1);
    }
  }

  for (int i = 0; i < contador; ++i) {
    pthread_join(hilos_cliente[i], NULL);
  }

  if (client_socket < 0) {
    perror("Fallo al aceptar la conexion con el cliente");
    exit(2);
  }

  fclose(listado);
  fclose(log_file);

  return 0;
}

void SIGN_HANDLER(int sig) {
  fclose(listado);
  fclose(log_file);
  shutdown(server_socket, SHUT_RDWR);
  printf("\nSERVIDOR FINALIZADO.");
  exit(0);
}

void server_log(size_t type, int socket, char* message) {
  pthread_mutex_lock(&log_lock);
    char log_header[1024];
    switch(type) {
      case CLIENT_CONNECTED:
        sprintf(log_header, "[NUEVO CLIENTE]: SOCKET %d", socket);
        break;
      case CLIENT_MESSAGE:
        sprintf(log_header, "[MENSAJE DE CLIENTE (SOCKET: %d)]", socket);
        break;
      case SERVER_MESSAGE:
        strcpy(log_header, "[MENSAJE DE SERVIDOR]: ");
        break;
      case CLIENT_DISCONNECTION:
        sprintf(log_header, "[CLIENTE DESCONECTADO]: SOCKET %d", socket);
        break;
    }

    if (message == NULL) {
      fprintf(log_file, "%s\n", log_header);
    } else {
      fprintf(log_file, "%s %s\n", log_header, message);
    }
  pthread_mutex_unlock(&log_lock);
}

void cargar_archivo(FILE** archivo, char* path, char* flag) {
  if (flag == NULL) {
    flag = "r";
  }
  *archivo = fopen(path, flag);

  if (*archivo == 0) {
    perror("Error al abrir archivos de listado o log.");
    exit(3);
  }
}

void *connection_handler(void *socket_desc) {
  int socket = *(int*)socket_desc;
  int read_size;

  int listado_alumno_cargado = 0;
  int cantidad_alumnos = 0;
  // TODO(TOMAS): Agregar esta info en el help.
  // La cantidad de alumnos por profesor no pueden ser mayor a 100.
  // y sus nombres no mayores a 100.
  char* alumnos[100];
  // Probably will need a malloc here :()
  int loop_alumnos = 0;

  int cargando_asistencia = 0;
  char archivo_asistencia[20];

  struct User *user;
  char *message, client_message[1025];

  message = (char*)(malloc(1025));

  server_log(CLIENT_CONNECTED, socket, NULL);

  strcpy(message, "Bienvenido a la plataforma de la Universidad Nacional de La Matanza\nIngrese nombre de usuario y contraseña como <nombre_de_usuario>:<contraseña>\nINGRESE: ");
  write(socket, message, strlen(message));
  server_log(SERVER_MESSAGE, 0, message);

  while ((read_size = recv(socket, client_message, 1025, 0)) > 0) {
    client_message[read_size] = '\0';
    char *newline = strchr(client_message, '\n' );
    if ( newline )
      *newline = 0;
    server_log(CLIENT_MESSAGE, socket, client_message);
    
    if (user == NULL) {
      char* username;
      char* password;
      username = strtok(client_message, ":");
      password = strtok(NULL, ":");

      if (password == NULL) {
        sprintf(message, "EL USUARIO SE ESPERA COMO USUARIO:CONTRASEÑA\n");
        write(socket, message, strlen(message));
        server_log(SERVER_MESSAGE, 0, message);
      } else {
        login(username, password, &user);
      }
      
      if (user != NULL) {
        if (user->role == 'D' && listado_alumno_cargado == 0) {
          traer_listado_alumnos_comision(user->comision, &(*alumnos), &cantidad_alumnos);
          listado_alumno_cargado == 1;
          sprintf(message, "\nBIENVENIDO %s | ROLE: %c | COMISION: %d\nIngrese una fecha como yyyy-mm-dd para ver el listado de presencia de la fecha o cargarlo en caso que no exista.\nFECHA: ", user->name, user->role, user->comision);
        } else if (user->role == 'A') {
          sprintf(message, "\nBIENVENIDO %s | ROLE: %c | COMISION: %d\nIngrese una fecha como yyyy-mm-dd para ver su precensia ese dia\no mande 'ASISTENCIA' para tener su porcentaje de asistencia actual\nMENSAJE: ", user->name, user->role, user->comision);
        }

        write(socket, message, strlen(message));
        server_log(SERVER_MESSAGE, 0, message);
      } else {
        strcpy(message, "\nNo se encontro usuario con esa combinacion de USUARIO:CONTRASEÑA\nINGRESE NUEVAMENTE: ");
        write(socket, message, strlen(message));
        server_log(SERVER_MESSAGE, 0, message);
      }
    } else {
      if (user->role == 'D') {
        if (cargando_asistencia == 1) {
          if (loop_alumnos == cantidad_alumnos - 1 && (*client_message == 'P' || *client_message == 'A')) {
            char* string_asistencia = (char*)(malloc(100));
            sprintf(string_asistencia, "%s|%c", alumnos[loop_alumnos], *client_message);
            cargar_asistencia(archivo_asistencia, string_asistencia);
            free(string_asistencia);
            strcpy(message, "\nFINALIZO EL PROCESO DE CARGA. SE HA GUARDADO EL ARCHIVO\nPuede volver a consultar ahora\nFECHA: ");
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
            cargando_asistencia = 0;
            loop_alumnos = 0;
          } else if (loop_alumnos != cantidad_alumnos - 1 && (*client_message == 'P' || *client_message == 'A')) {
            char* string_asistencia = (char*)(malloc(100));
            sprintf(string_asistencia, "%s|%c", alumnos[loop_alumnos], *client_message);
            cargar_asistencia(archivo_asistencia, string_asistencia);
            loop_alumnos++;
            sprintf(message, "Ingrese (P)resente o (A)usente para \"%s\": ", alumnos[loop_alumnos]);
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
            free(string_asistencia);
          } else {
            sprintf(message, "\nERROR: Se esperaba 'P' para presente, o 'A' para ausente.\n\nIngrese (P)resente o (A)usente para \"%s\": ", alumnos[loop_alumnos]);
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
          }
        } 
        else if (fecha_valida(client_message) == 0) {
          strcpy(message, "ERROR: Se esperaba una fecha valida como yyyy-mm-dd como 2020-12-31, intente nuevamente\nFECHA: ");
          write(socket, message, strlen(message));
          server_log(SERVER_MESSAGE, 0, message);
        } else if (cantidad_alumnos == 0) {
          strcpy(message, "\nEl docente no tiene alumnos a cargo en su comision.\nNo va a poder realizar cargas ni consultas. Asegurese de que en el archivo de listado esten ingresados sus alumnos!\nPuede volver a consultar ahora\nFECHA: ");
          write(socket, message, strlen(message));
          server_log(SERVER_MESSAGE, 0, message);
        } else {
          sprintf(archivo_asistencia, "Asistencia_%s_%d.txt", client_message, user->comision);

          if (check_for_file(archivo_asistencia) == 1) {
            char *file_name = (char*)(malloc(50));
            sprintf(file_name, "./Asistencia/%s", archivo_asistencia);

            strcpy(message, "\nNOMBRE|PRESENTE\n");
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);

            char* line;
            ssize_t read;
            ssize_t len = 0;
            pthread_mutex_lock(&acceso_a_archivo);
            FILE *f = fopen(file_name, "r");
            while((read = getline(&line, &len, f) != -1)) {
              sprintf(message, "%s", line);
              write(socket, message, strlen(message));
              server_log(SERVER_MESSAGE, 0, message);
            }
            
            sprintf(message, "\nPuede volver a consultar ahora\nFECHA: ");
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);

            fclose(f);
            pthread_mutex_unlock(&acceso_a_archivo);
            free(line);
            free(file_name);
          } else {
            sprintf(message, "No hay un archivo cargado para esa fecha\nCARGA DE PRESENCIA INICIADO\nIngrese (P)resente o (A)usente para \"%s\": ", alumnos[loop_alumnos]);
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
            cargando_asistencia = 1;
          }
        }
        // El docente podra ingresar una fecha (yyyy-mm-dd) y se debera
        // ver si existe el archivo de asistencias (Asistencias_[FECHA]_[COMISION]) para ese
        // dia para mostrarlo.
        // Si no existe se debera promptear para que ingrese
        // Mostrandole previamente cuales son los Alumnos que tiene asignados
        // en su comision y debera llenar el listado hasta que mande un 'FIN' y se guardara el archivo.
      } else {
        // El alumno podra ingresar una fecha y recibir si
        // estuvo presente ese dia enviando una fecha como 'yyyy-mm-dd'
        // y buscarlo en un archivo Asistencias_[FECHA]_[COMISION]
        // y si envia 'ASISTENCIA' se debera recorrer todos los
        // archivos que tengan Asistencias_CUALQUIERCOSA_[COMISION]
        // e ir viendo si tienen el nombre del alumno y su estado (A o P)
        // y en base al total de Ausentes y Presentes mostrar el porcentaje
        // de asistencia por el momento.$
        if (strcmp(client_message, "ASISTENCIA") == 0) {
          int porcentaje_asistencia = calcular_porcentaje_asistencia(user->name, user->comision);
          sprintf(message, "\nEl usuario %s de la comision %d tiene un porcentaje de asistencia del %d%%\n\nPuede volver a consultar ahora\nMENSAJE: ", user->name, user->comision, porcentaje_asistencia);
          write(socket, message, strlen(message));
          server_log(SERVER_MESSAGE, 0, message);
        } else if (fecha_valida(client_message) == 1) {
          sprintf(archivo_asistencia, "Asistencia_%s_%d.txt", client_message, user->comision);
          
          if (check_for_file(archivo_asistencia) == 1) {
            if (asistio(archivo_asistencia, user->name) == 1) {
              sprintf(message, "\nEl dia de la fecha %s el alumno %s ASISTIO a la clase de la comision %d\n\nPuede volver a consultar ahora\nMENSAJE: ", client_message, user->name, user->comision);
              write(socket, message, strlen(message));
              server_log(SERVER_MESSAGE, 0, message);
            } else {
              sprintf(message, "\nEl dia de la fecha %s el alumno %s SE AUSENTO a la clase de la comision %d\n\nPuede volver a consultar ahora\nMENSAJE: ", client_message, user->name, user->comision);
              write(socket, message, strlen(message));
              server_log(SERVER_MESSAGE, 0, message);
            }
          } else {
            sprintf(message, "\nNo se encontro archivo de asistencias para la fecha %s y la comision %d\n\nPuede volver a consultar ahora\nMENSAJE: ", client_message, user->comision);
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
          }
        } else {
            strcpy(message, "\nERROR: Se esperaba una fecha valida como yyyy-mm-dd para ver sus asistencia ese dia o el mensaje 'ASISTENCIA' para tener su porcentaje de asistencia actual.\n\nPuede volver a consultar ahora\nMENSAJE: ");
            write(socket, message, strlen(message));
            server_log(SERVER_MESSAGE, 0, message);
        }
      }
    }

    memset(client_message, 0, 1025);
  }

  if (read_size == 0) {
    server_log(CLIENT_DISCONNECTION, socket, NULL);
    pthread_exit(NULL);
  } else if (read_size == -1) {
    perror("Fallo al recibir informacion (recv failed)");
  }

  return 0;
}

void login(char* username, char* password, struct User **user) {
  size_t buffer_size = 255;
  char line_buffer[buffer_size];

  fseek(listado, 0, SEEK_SET);

  while (fgets(line_buffer, buffer_size, listado)) {
    char* usr = strtok(line_buffer, "|");
    char* pwd = strtok(NULL, "|");
    char* role = strtok(NULL, "|");
    char* com = strtok(NULL, "|");

    if ((strcmp(username, usr) == 0) && (strcmp(password, pwd) == 0)) {
      *user = (struct User*)(malloc(sizeof **user));
      strcpy((*user)->name, usr);
      (*user)->role = *role;
      (*user)->comision = atoi(com);
      return;
    }
  }

  return;
}

int check_for_file(char* file_name) {
  DIR *d;
  struct dirent *dir;

  pthread_mutex_lock(&acceso_a_archivo);
  d = opendir("./Asistencia");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (strcmp(file_name, dir->d_name) == 0) {
        closedir(d);
        pthread_mutex_unlock(&acceso_a_archivo);
        return 1;
      } 
    }
    closedir(d);
    pthread_mutex_unlock(&acceso_a_archivo);
  }
  return 0;
}

void cargar_asistencia(char *archivo, char *contenido) {
  char* path;
  sprintf(path, "./Asistencia/%s", archivo);
  pthread_mutex_lock(&acceso_a_archivo);
  FILE *f = fopen(path, "a+");
  fprintf(f, "%s\n", contenido);
  fclose(f);
  pthread_mutex_unlock(&acceso_a_archivo);
}

int asistio(char* archivo, char* user) {
  size_t buffer_size = 255;
  char line_buffer[buffer_size];
  char* file_path = (char*)(malloc(100));

  sprintf(file_path, "./Asistencia/%s", archivo);

  pthread_mutex_lock(&acceso_a_archivo);
  FILE* f = fopen(file_path, "r");
  fseek(f, 0, SEEK_SET);
  free(file_path);

  while (fgets(line_buffer, buffer_size, f)) {
    char* usr = strtok(line_buffer, "|");
    char* asistencia = strtok(NULL, "|");
    if (asistencia == NULL) {
      continue;
    }

    char *newline = strchr(asistencia, '\n' );
    if ( newline )
      *newline = 0;

    if ((strcmp(user, usr) == 0) && (strcmp(asistencia, "P") == 0)) {
      pthread_mutex_unlock(&acceso_a_archivo);
      return 1;
    }
  }
  fclose(f);
  pthread_mutex_unlock(&acceso_a_archivo);
  return 0;
}

int calcular_porcentaje_asistencia(char* usuario, int comision) {
  DIR *d;
  struct dirent *dir;

  int cantidad_dias = 0;
  int cantidad_presentes = 0;

  pthread_mutex_lock(&asistencia_mutex);
  d = opendir("./Asistencia");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      char* _ = strtok(dir->d_name, "_");
      char* fecha = strtok(NULL, "_");
      char* comisionConExtension = strtok(NULL, "_");
      char* com = strtok(comisionConExtension, ".");

      if (!comisionConExtension) {
        continue;
      }
      int comisionArchivo = atoi(comisionConExtension);

      if (comision == comisionArchivo) {
        cantidad_dias++;
        char* nombre_archivo = (char*)(malloc(50));
        sprintf(nombre_archivo, "Asistencia_%s_%d.txt", fecha, comisionArchivo);
        if (asistio(nombre_archivo, usuario) == 1) {
          cantidad_presentes++;
        }
        free(nombre_archivo);
      } 
    }
    closedir(d);
    pthread_mutex_unlock(&asistencia_mutex);
  }

  if (cantidad_dias == 0) {
    return 0;
  }
  
  return cantidad_presentes * 100 / cantidad_dias;
}

void traer_listado_alumnos_comision(int comision, char *listado_alumnos[100], int* cantidad_alumnos) {
  size_t buffer_size = 255;
  char line_buffer[buffer_size];

  fseek(listado, 1, SEEK_SET);

  while (fgets(line_buffer, buffer_size, listado)) {
    char* usr = strtok(line_buffer, "|");
    char* pwd = strtok(NULL, "|");
    char* role = strtok(NULL, "|");
    char* com = strtok(NULL, "|");

    char *newline = strchr(com, '\n' );
    if ( newline ) {
      *newline = 0;
    }

    if (strcmp(com, "COD_COMISION") == 0) {
      continue;
    }

    if (atoi(com) == comision && *role != 'D') {
      if (*cantidad_alumnos == 100) {
        // No puede tener mas de 100 alumnos!!
        return;
      }
      listado_alumnos[*cantidad_alumnos] = (char*)(malloc(100));
      strcpy(listado_alumnos[*cantidad_alumnos], usr);
      (*cantidad_alumnos)++;
    }
  }
}

int fecha_valida(char* fecha_string) {
  char* fecha_mutable = (char*)(malloc(100));
  strcpy(fecha_mutable, fecha_string);
  char* year = strtok(fecha_mutable, "-");
  char* month = strtok(NULL, "-");
  char* day = strtok(NULL, "-");

  if (year == NULL || month == NULL || day == NULL) {
    free(fecha_mutable);
    return 0;
  }

  if (atoi(year) < 1900 || atoi(year) > 2100) {
    free(fecha_mutable);
    return 0;
  }

  if (atoi(month) < 1 || atoi(month) > 12) {
    free(fecha_mutable);
    return 0;
  }

  if (atoi(day) < 1 || atoi(day) > 31) {
    free(fecha_mutable);
    return 0;
  }

  free(fecha_mutable);
  return 1;
}