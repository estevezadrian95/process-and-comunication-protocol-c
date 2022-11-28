#include "leeLinea.h"
#include "sharedmem.h"
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
#define ASISTENCIA_LEN 256
int memoryID;
wrapper *memory;
int rc;
Mensaje pagos[ASISTENCIA_LEN];
int main(int argc, char** argv)
{
    //printf("Soy el productor de pagos\n");
    if (argc < 2) {
        printf("uso: %s <nombre archivo>\n", argv[0]);
        exit(1);
    }
    char *tmp;
    char* fileName = argv[1];

    FILE* fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("el archivo %s no existe\n", fileName);
        exit(1);
    }
    memory = obtieneMemoriaCompartida(0666);
    char* linea = leeLinea(fp);
    int i = 0;

    while(linea != NULL) {
        sem_wait(&memory->empty);
        sem_wait(&memory->pmutex);

        Mensaje pago = pagos[i];
        pago.Tipo = 2;      
        tmp = strtok(linea, ";");
        pago.Dni = atoi(tmp);
        tmp = strtok(NULL, ";");
        strcpy(pago.DiaFecha,tmp);
        i++;

        memory->n++;
        Mensaje* mensaje = &((memory->array)[memory->n]);
        memcpy(mensaje, &pago, sizeof(Mensaje));
        //printf("--pp-- %d indice, TIPO: %d DNI: %d, FECHA: %s\n", memory->n, mensaje->Tipo, mensaje->Dni, mensaje->DiaFecha);

        linea = leeLinea(fp);
        if(linea == NULL)
        {
            memory->terminado++;
            //printf("--Termino el productor de pagos--\n");
        }
         
        sem_post(&memory->pmutex);
        sem_post(&memory->full);
    }
    fclose(fp);
    exit(0);
}