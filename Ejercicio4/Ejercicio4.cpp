#include <iostream>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int bandera_c, bandera_r, bandera_p;
pid_t pid_control;
pid_t pid_registro;
pid_t pid_principal;

int linea(int df, char* cad)
{
        int n;

        do{
                n = read(df, cad, 1);
        }while( n > 0 && *(cad++) );

        return (n > 0);
}

void controlador (int sig)
{
        kill(pid_control, SIGUSR1);
        kill(pid_registro, SIGUSR1);
        bandera_p = 0;
}

void controlador_c (int sig)
{
        bandera_c = 0;
}

void controlador_r (int sig)
{
        bandera_r = 0;
}

bool ayuda(char*cad)
{
        if( strcmp(cad,"-help") == 0 || strcmp(cad,"-h") == 0 || strcmp(cad,"-?") == 0 )
                return true;
        else
                return false;
}

int main(int nargs, char *args[])
{
    if( nargs == 2 && ayuda(args[1]) ) {
        puts(
        "Ayuda:\
        \n\n\tTRABAJO PRÁCTICO 3 - EJERCICIO 4\
        \n\tINTEGRANTES:\
        \n\tParra, Martín                  DNI:40012233\
        \n\tDi Vito, Tomás                 DNI:39340228\
        \n\tFernández, Matías Gabriel      DNI:38613699\
        \n\tMiró, Agustín                  DNI:40923621\
        \n\tEstevez, Adrián                DNI:39325872\
        \n\n\tEste script se encarga de detectar fugas de memoria y cpu.\
        \n\tLos límites a superar deben ser pasados por parámetro indicando el límite para cpu y memoria respectivamente.\
        \n\tPara finalizar la ejecución, se debe enviar la señal SIGUSR1 al proceso principal, cuyo PID será informado al iniciar la ejecución.\
        \n\tEl resultado del análisis se irá grabando en un archivo llamado log.txt ubicado en el mismo directorio que el script.\
        \n\n\tFORMA DE EJECUCION:\
        \n\t./servidor [límite CPU] [límite memoria]\
        \n\n\tEJEMPLO:\
        \n\t./servidor 25 34\n");
        return 0;
    }
    else if( nargs != 3 ) {
        cout << "No se ingreso la cantidad correcta de parametros." << endl;
        exit(0);
    }

    unlink("canal");
    mkfifo("canal",0);
    chmod("canal",460);

    pid_control = fork();

    if( pid_control == 0 ) {
        int descr_c, longproc;
        char msj[140];
        char proceso[8];
        int procesos[100000] = {0};
        int campo = 1;
        float cpu_f;
        float mem_f;
        int estado;
        char cmd[200];
        FILE *fp;
        FILE *fp_cpu;
        FILE *fp_mem;
        bandera_c = 1;

        signal(SIGUSR1, controlador_c);

        do{
                descr_c = open("canal", O_WRONLY);
                if ( descr_c == -1 )
                        sleep(1);
        }while( descr_c == -1 );

        while( bandera_c ) {
                fp = popen("ps -exo pid | awk -F ' ' 'NR>1'", "r");

                campo = 1;
                while( fgets(proceso, sizeof(proceso), fp) != NULL ) {
                        strtok(proceso, "\n"); //elimina la contrabarra n

                        sprintf(cmd, "ps -p %s -o %%cpu | awk 'NR>1'", proceso);
                        fp_cpu = popen(cmd, "r");
                        fread(cmd,1,200,fp_cpu);
                        fclose(fp_cpu);
                        cpu_f=(float)atof(cmd);
                        memset(cmd, 0, sizeof cmd);

                        sprintf(cmd, "ps -p %s -o %%mem | awk 'NR>1'", proceso);
                        fp_mem = popen(cmd, "r");
                        fread(cmd,1,200,fp_mem);
                        fclose(fp_mem);
                        mem_f=(float)atof(cmd);
                        memset(cmd, 0, sizeof cmd);

                        estado = 0;
                        if( cpu_f > (float)atof(args[1]) )
                                estado = 1;
                        if( mem_f > (float)atof(args[2]) )
                                if( estado == 0 )
                                        estado = 2;
                                else
                                        estado = 3;

                        if( procesos[atoi(proceso)] != 3 && procesos[atoi(proceso)] != estado && estado != 0 ) {
                                procesos[atoi(proceso)] = estado;
                                if( estado == 1 ) {
                                        strcat(msj, proceso);
                                        strcat(msj, ": Supera CPU");
                                }
                                else if( estado == 2) {
                                        strcat(msj, proceso);
                                        strcat(msj, ": Supera memoria");
                                }
                                else {
                                        strcat(msj, proceso);
                                        strcat(msj, ": Supera ambos");
                                }
                                longproc = strlen(msj)+1;
                                write( descr_c, msj, longproc);
                        }
                        memset(msj, 0, sizeof msj);
                        campo++;
                }
                fclose(fp);
                sleep(1);
        }
        close(descr_c);
        exit(0);
    }

    pid_registro = fork();

    if( pid_registro == 0 ) {
        int descr_r;
        char mensaje[140], date[64];
        time_t t = time(NULL);
        struct tm *tm;
        FILE *flog;

        bandera_r = 1;

        //flog = fopen("log.txt","a+");
        fclose(fopen("log.txt", "w"));

        signal(SIGUSR1, controlador_r);

        descr_r = open("canal", O_RDONLY);

        while( bandera_r )
                if( linea( descr_r, mensaje ) ) {
                        tm = localtime(&t);
                        strftime(date, sizeof(date), "%c", tm);
                        flog = fopen("log.txt","a+");
                        fprintf(flog, "[%s] %s\n", date,mensaje);
                        fclose(flog);
                }

        //fclose(flog);
        close(descr_r);
        remove("canal");

        exit(0);
    }

    pid_principal = fork();
    if( pid_principal == 0 ) {
        bandera_p = 1;
        signal(SIGUSR1, controlador);
        while( bandera_p );
        exit(0);
    }

    cout << "ID del control: " << pid_control << endl;
    cout << "ID del registro: " << pid_registro << endl;
    cout << "ID del proceso principal: " << pid_principal << endl;
    return 0;
}