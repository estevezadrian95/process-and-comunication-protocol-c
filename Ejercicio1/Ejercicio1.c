#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define ARRAY_SIZE 10


struct nodo
{
    int nro;
    int id_padre;
    int id_hijo;
};

void ayuda();
void error();
void guardar_info(int ,pid_t);
int crear_procesos(int n);
void mostrar_info();
int validaciones(int argc, char *argv[]);

struct nodo identificadores[ARRAY_SIZE];

int main(int argc, char *argv[])
{
    int numero;
    numero = validaciones(argc,argv);
    if(numero < 0)
    {
        exit(-1);
    }
    else
    {
        crear_procesos(numero);
        exit(0);
    }
}

int crear_procesos(int n)
{
    int estado;
    guardar_info(0,getpid());
    mostrar_info();

    pid_t h1 = fork();
    if(h1==0)
    {
        guardar_info(1,getpid());
        mostrar_info();
        exit(0);
    }
    else if(h1>0)
    {   
        
        pid_t h2 = fork();
        
        if (h2 == 0)
        {
            guardar_info(2,getpid());
            mostrar_info();

            pid_t n4 = fork();
            if(n4==0)
            {
                guardar_info(4,getpid());
                mostrar_info();

                pid_t bn7 = fork();
                if(bn7==0)
                {
                    guardar_info(7,getpid());
                    mostrar_info();

                    for(int i=1;i<=n;i++)
                    {
                        pid_t numero = fork();
                        if(numero>0) // Si es el padre Sale ya que el hijo tiene que seguir procreando
                        {
                            break;
                        }
                        else
                        {
                            guardar_info(7+i,getpid());
                            mostrar_info();
                        }
                        
                    }    
                    exit(0);
                }                
                
            }else if(n4>0)
            {
                pid_t n5 = fork();
                if(n5==0)
                {
                    guardar_info(5,getpid());
                    mostrar_info();
                    exit(0);
                }else
                {
                    exit(0);    
                }
            }
        exit(0);
        }
        else //Hijo 3
        {
            pid_t h3 = fork();
            if (h3==0)
            {
                guardar_info(3,getpid());
                mostrar_info();
                
                pid_t n6 = fork();
                if(n6==0)
                {
                    guardar_info(6,getpid());
                    mostrar_info();
                    exit(0);
                }
            exit(0);
            }
        exit(0);
        }
    }
}

void guardar_info(int nro, int pid)
{
    struct nodo n;
    n.id_padre = pid;
    n.nro = nro;

    identificadores[nro] = n;
}

void mostrar_info()
{
    int ultimo_ID = 0;
    
    for(int i=sizeof(identificadores);i>0;i--)
    {
        if(identificadores[i].id_padre!=0 && identificadores[i].nro != 0)
        {
            printf("%01d(%04d) - ", identificadores[i].nro,identificadores[i].id_padre);
        }
    }
     printf("%01d(%04d)\n\n", 0,identificadores[0].id_padre);
}
    
int validaciones(int argc, char *argv[])
{
    int n;
    if(argc<=1)
    {
        printf("No ha ingresado ningun numero, por favor revise la ayuda\n");
        exit(-1);
    } 
    else if(argc>2)
    {
        printf("Ingreso mas de un parametro, por favor revise la ayuda\n");
        exit(-1);
    }
    else if(strcmp(argv[1],"-h")==0||strcmp(argv[1],"-help")==0||strcmp(argv[1],"-?")==0)
    {
        ayuda();
        exit(-1);
    }
    else if(atoi(argv[1])<0)
    {
        printf("ingreso un numero negativo, por favor revise la ayuda\n");
        exit(-1);
    }
    else if(!isdigit(*argv[1]))
    {
        printf("Lo que ingreso no es un numero valido, por favor revise la ayuda\n");
        exit(-1);
    }
    n=atoi(argv[1]);
    if(n<0)
    {
        printf("ERROR, ha ingresado un numero negativo\n");
        exit(-1);
    }
    
return n;
}    

void ayuda()
{
    printf("Ingrese un numero N para crear N jerarquias de procesos a partir del subproceso 7\n");
    printf("Ejemplo de ejecucion:\n");
    printf("Paso 1: 'make'\n");
    printf("Paso 2: './e01.o 3'\n\n");
    
    printf("TRABAJO PRÁCTICO 3 - EJERCICIO 1\n");    
    printf("INTEGRANTES:\n");
    printf("Parra, Martin                  DNI:40012233\n");
    printf("Di Vito, Tomas                 DNI:39340228\n");
    printf("Fernandez, Matias Gabriel      DNI:38613699\n");
    printf("Mirò, Agustin                  DNI:40923621\n");
    printf("Estevez, Adrian                DNI:39325872\n");
}
