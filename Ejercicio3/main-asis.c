#include "leeLinea.h"
#include "lista.h"
#include <string.h>
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "sharedmem.h"
#define ASISTENCIA_LEN 256

Mensaje asistencias[ASISTENCIA_LEN];
wrapper *memory;


int main(int argc, char** argv)
{
    // printf("Soy el productor de asistencias\n");
    if (argc < 2) {
        printf("uso: %s <nombre archivo>\n", argv[0]);
        return 1;
    }
    char *tmp;
    char* fileName = argv[1];

    FILE* fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("el archivo %s no existe\n", fileName);
        return 1;
    }

    memory = obtieneMemoriaCompartida(0666);

    char* linea = leeLinea(fp);
    int i = 0;

    while(linea != NULL) {
        sem_wait(&memory->empty);
        sem_wait(&memory->pmutex);

        Mensaje asistencia = asistencias[i];
        asistencia.Tipo = 1;      
        tmp = strtok(linea, ";");
        asistencia.Dni = atoi(tmp);
        tmp = strtok(NULL, ";");
        strcpy(asistencia.DiaFecha,tmp);
        i++;

        memory->n++;
        Mensaje* mensaje = &((memory->array)[memory->n]);
        memcpy(mensaje, &asistencia, sizeof(Mensaje));

        //printf("--pa--%d indice, TIPO: %d DNI: %d, DIA SEMANA: %s\n", memory->n, mensaje->Tipo, mensaje->Dni, mensaje->DiaFecha);
        
        linea = leeLinea(fp);
        if(linea == NULL)
        {
            memory->terminado++;
            //printf("--Termino el productor de asistencias--\n");
        } 

        sem_post(&memory->pmutex);
        sem_post(&memory->full);
    }
    fclose(fp);
    exit(0);
}