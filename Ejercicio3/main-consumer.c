#include "leeLinea.h"
#include "sharedmem.h"
#include "lista.h"
#include <string.h>
#include <fcntl.h>
#include <time.h>
#define ASISTENCIA_LEN 256
typedef struct 
{
    char Nombre[32];
    char Apellido[32];
    int Dni;
    char NombreDelDeporte[16];
    char DiaAsistencia[11];
} Socio;
typedef struct 
{
    char NombreDelDeporte[16];
    char DiasPosibles[21];
    int ValorCuota;

} Deporte;
// Nombre;Apellido;DNI;NombreDelDeporte;DíaAsistencia
int FechaValida(char fecha[10], int *dia);
Mensaje* ObtenerPagoPorDni(int dni, int* dia);
Deporte* ObtenerValorCuotaDeporte(char* deporte, char* diasDisponibles);
void ObtenerAsistenciasPorDni(int dni, char* diasDisponibles, int pago);

int memoryID;
wrapper *memory;
int rc;
Mensaje asistencias[ASISTENCIA_LEN];
Mensaje pagos[ASISTENCIA_LEN];
Socio socios[ASISTENCIA_LEN];

Deporte deportes[] =
{
    {"Futbol","Lunes - Miercoles",1000},
    {"Voley","Martes - Jueves",1200},
    {"Basquet","Viernes",1300},
    {"Natacion","Sabado",1800}
};

void atexit_function() {
    printf("Liberando Recursos...\n");
    rc = shmctl(memoryID, IPC_RMID, NULL);
    rc = shmdt(memory);
    sem_destroy(&memory->pmutex);
    sem_destroy(&memory->empty);
    sem_destroy(&memory->full);
    printf("Recursos Liberados.\n");
}

int memoryID;
wrapper *memory;
int rc;
int currentMonth;
int currentDay;
int currentYear;
int main(int argc, char** argv)
{
    time_t s = time(NULL);
    struct tm* current_time = localtime(&s);
    currentMonth = (current_time->tm_mon + 1);
    currentDay = (current_time->tm_mday);
    currentYear = (current_time->tm_year + 1900);
    
    atexit(atexit_function);
    printf("\n");

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

    int i = 0;
    char* linea = leeLinea(fp);
    while(linea != NULL) {
        tmp = strtok(linea, ";");
        strcpy(socios[i].Nombre,tmp);
        tmp = strtok(NULL, ";");
        strcpy(socios[i].Apellido,tmp);    
        tmp = strtok(NULL, ";");
        socios[i].Dni = atoi(tmp);
        tmp = strtok(NULL, ";");
        strcpy(socios[i].NombreDelDeporte,tmp);
        tmp = strtok(NULL, ";");
        strcpy(socios[i].DiaAsistencia,tmp);
        Socio socio = socios[i];
        free(linea);
        //printf("%d indice, NOMBRE: %s, APELLIDO: %s, DNI: %d, NOMBREDEPORTE: %s, DIA SEMANA: %s\n", i, socio.Nombre, socio.Apellido, socio.Dni, socio.NombreDelDeporte, socio.DiaAsistencia);
        i++;
        linea = leeLinea(fp);
    }
    fclose(fp);
    //creating key for shared memory
    memory = obtieneMemoriaCompartida(0666);
    i = 0;
    int j = 0;
    while(1) {
        sem_wait(&memory->full);
        sem_wait(&memory->pmutex);

        int n = memory->n;
        Mensaje temp = (memory->array)[n];
        int tipo = temp.Tipo;

        if(tipo == 1)
        {
            asistencias[i].Tipo = tipo;
            asistencias[i].Dni = temp.Dni;
            strcpy(asistencias[i].DiaFecha,temp.DiaFecha);
            i++;
        }
        if(tipo == 2)
        {
            pagos[j].Tipo = tipo;
            pagos[j].Dni = temp.Dni;
            strcpy(pagos[j].DiaFecha,temp.DiaFecha);
            j++;
        }
        // printf("--c-- %d indice, TIPO: %d DNI: %d, DIA SEMANA: %s\n", memory->n, temp.Tipo, temp.Dni, temp.DiaFecha);
        memory->n--;
        if(memory->n == -1 && memory->terminado == 2)
        {
            sem_post(&memory->pmutex);
            sem_post(&memory->empty);
            break;
        }
        
        sem_post(&memory->pmutex);
        sem_post(&memory->empty);
    }
    // Aca va la logica de generar el reporte
    float totalRecaudado = 0;
    for(int n=0; n < NELEMS(socios); n++)
    {
        char* diasDisponiblesDeporte;
        int aplicaDescuento = 0;
        Socio socio = socios[n];
        if(socio.Dni == 0) break;
        Deporte* deporte = ObtenerValorCuotaDeporte(socio.NombreDelDeporte, diasDisponiblesDeporte);
        float cuotaDeporte;
        cuotaDeporte = deporte->ValorCuota; 
        if(deporte == NULL)
        {
            printf("No se pudo encontrar el deporte para el socio %d\n", socio.Dni);
            continue;
        }
        if(strstr(deporte->DiasPosibles,socio.DiaAsistencia) == NULL)
        {
            printf("El socio dni: %d tiene configurado el dia %s cuando solo podia el/los dia/s %s\n", socio.Dni, socio.DiaAsistencia, deporte->DiasPosibles);
            continue;
        }
                
        int dia = 0;
        Mensaje* pago = ObtenerPagoPorDni(socio.Dni, &dia);
        if(pago)
        {           
            if(dia < 11)
            {
                aplicaDescuento = 1;  
            }
            float montoPagado = aplicaDescuento ? cuotaDeporte * 0.9 : cuotaDeporte;
            totalRecaudado += montoPagado;
        } else printf("No se encontro pago para el socio dni %d\n", socio.Dni);
        ObtenerAsistenciasPorDni(socio.Dni, socio.DiaAsistencia, pago ? 1 : 0);        
    }
    printf("El monto total recaudado es %2f\n", totalRecaudado);
    printf("Se termino el reporte.\n");
    exit(0);
}
Deporte* ObtenerValorCuotaDeporte(char* deporte, char* diasDisponibles)
{
    for(int n=0; n < NELEMS(deportes); n++)
    {
        if(strcmp(deportes[n].NombreDelDeporte, deporte) == 0)
        {
            return &deportes[n];
        }
    }
    return 0;
}

Mensaje* ObtenerPagoPorDni(int dni, int* dia)
{
    for(int n=0; n < NELEMS(pagos); n++)
    {
        
        Mensaje pago = pagos[n];
        if(pago.Tipo == 0) break;
        if(pago.Dni == dni && FechaValida(pago.DiaFecha, dia))
        {
            return &pagos[n];
        }
    }
    return NULL;
}
void ObtenerAsistenciasPorDni(int dni, char* diasDisponibles, int pago)
{
    int asistio = 0;
    for(int n=0; n < NELEMS(asistencias); n++)
    {
        Mensaje asistencia = asistencias[n];
        if(asistencia.Tipo == 0) break;

        if(asistencia.Dni == dni)
        {
            asistio = 1;
            if(strcmp(asistencia.DiaFecha, diasDisponibles) != 0)
            {
                printf("El socio dni: %d asistio el dia %s cuando solo podia el dia %s\n", dni, asistencia.DiaFecha, diasDisponibles);
                if(!pago) printf("ademas, el socio no pago la cuota mensual.\n");
            }
            else if(!pago)
            {
                printf("El socio dni: %d asistio el dia que debia pero no pago.\n", dni);
            }
        }
    }
    if(!asistio && pago)
    {
        printf("El socio dni: %d no asistio pero pago.\n", dni);
    }
}
int FechaValida(char fecha[10], int *dia)
{
    int i;
    char *tmp;
    char* pfecha = fecha;
    for (i=0; pfecha[i]; pfecha[i]=='-' ? i++ : *pfecha++);

    if(i == 2)
    {
        tmp = strtok(fecha, "-");
        int anio = atoi(tmp);
        if(anio != currentYear)
            return 0;

        tmp = strtok(NULL, "-");
        int mes = atoi(tmp);
        if(mes != currentMonth)
            return 0;

        tmp = strtok(NULL, "-");
        int diaPago = atoi(tmp);
        if(diaPago < 0 || diaPago > 31)
            return 0;     
        *dia = diaPago;
        return 1;
    }
    return 0;
}