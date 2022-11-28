#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sharedmem.h"

int main(int argc, char** argv)
{
    if(argc == 2 && (!strcmp(argv[1],"-h")  || !strcmp(argv[1],"-?") || !strcmp(argv[1],"--help")))
    {
        printf("TRABAJO PRÁCTICO 3 - EJERCICIO 3\nINTEGRANTES:\n");
        printf("Parra, Martin                  DNI:40012233\n");
        printf("Di Vito, Tomas                 DNI:39340228\n");
        printf("Fernandez, Matias Gabriel      DNI:38613699\n");
        printf("Miro, Agustin                  DNI:40923621\n");
        printf("Estevez, Adrian                DNI:39325872\n");
        printf("Este programa es el launcher que generara los reportes de los archivos basandose en los archivos\nsocios, pagos y asistencia.\nNo necesita parametros\n");
        exit(1);
    }
    int memoryID;
    wrapper *memory;
    int rc;
    memory = obtieneMemoriaCompartida(IPC_CREAT | 0600);
    if(memory == NULL)
    {
        printf("No se pudo crear la memoria. se termina el programa\n");
        exit(1);
    }
    printf("Inicializando Recursos...\n");
    memset(&memory->array, 0, sizeof(memory->array));
    sem_init(&memory->pmutex, 1, 1);
    sem_init(&memory->empty, 1, SIZE_OF_ARRAY);
    sem_init(&memory->full, 1, 0);
    memory->n = -1;
    memory->terminado = 0;

    char * argv_prod_asis[] = {"productor-asis.bin","asistencia",NULL};
    char * argv_prod_pagos[] = {"productor-pagos.bin","pagos",NULL};
    char * argv_consumidor[] = {"consumer.bin","socios",NULL};
    char ** procesos[] = {argv_prod_asis, argv_prod_pagos,argv_consumidor};
    printf("Lanzando Procesos...\n");
    for(int i = 0; i < NELEMS(procesos); i++)
    {
        int pid = fork();
        if(pid == 0) { 
            execv(procesos[i][0],procesos[i]);
        }
    }
    printf("Procesos Lanzados.\n");
    //execv(argv_prod_asis[0],argv_prod_asis);
    //execv(argv_prod_pagos[0],argv_prod_pagos);
    //execv(argv_consumidor[0],argv_consumidor);
    exit(0);
}