#include "common.h"
#include <stdio.h>
wrapper *memory;
wrapper* obtieneMemoriaCompartida(int shmflag)
{
    int memoryID;
    key_t sharedMemoryKey = ftok(".", MEMORY_KEY);
    if (sharedMemoryKey == -1) {
        perror("ftok():");
        return NULL;
    }

    memoryID = shmget(sharedMemoryKey, sizeof(wrapper), shmflag);
    if (memoryID == -1) {

        perror("shmget():");
        return NULL;
    }

    memory = shmat(memoryID, NULL, 0);
    if (memory == (void *) -1) {
        perror("shmat():");
        return NULL;
    }
    return memory;
}