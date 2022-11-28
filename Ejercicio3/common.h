#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define MEMORY_KEY 11
#define SIZE_OF_ARRAY 10
#define SIZE_OF_FECHA 10
typedef struct 
{
    int Tipo;
    int Dni;
    char DiaFecha [10];

} Mensaje;
typedef struct
{
    Mensaje array[SIZE_OF_ARRAY];
    sem_t empty;
    sem_t pmutex;
    sem_t full;
    int n;
    int terminado;
} wrapper;