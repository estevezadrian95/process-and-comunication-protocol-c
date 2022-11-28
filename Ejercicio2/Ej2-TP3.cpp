/*
#TP3 - Ejercicio 2
#Primer Entrega
#INTEGRANTES:
# Parra, Martin Ezequiel - DNI: 40012233
# Miro, Agustin - DNI: 40923621
# Estevez, Adrian - DNI: 39325872
# Di Vito, Tomas - DNI: 39340228    
# Fernandez, Matias - DNI: 38613699
*/

#include<thread>
#include<vector>
#include<iostream>
#include<cstring>
#include<cstdlib>
using namespace std;


//===========================================================================
//DEFINICION DE VARIABLES GLOBALES
//===========================================================================

vector<long double> vFibo;
vector<long double> sumProd(2); // Declaracion del vector.
int cantThreads;
vector<thread> myThreads; // Declaracion del vector.

//===========================================================================
//FUNCIONES
//===========================================================================
void ayuda()		
{
	cout<<"AYUDA PARA EJECUTAR EL PROGRAMA Ej2-TP3"<<endl;
	cout<<"NOMBRE"<<endl;
	cout<<"\tEjercicio 2"<<endl;
	cout<<"DESCRIPCION"<<endl;
	cout<<"\tEl programa realiza la sucesion de Fibonacci utilizando varios threads."<<endl; 
	cout<<"SYNOPSIS"<<endl;
	cout<<"\tEjecutar el Ej2-TP3 con los siguientes parametros"<<endl;
	cout<<"\tPrimer parametro: Nivel de paralelismo representado por un numero entero, que designara la cantidad de hilos a trabajar"<<endl;
	cout<<"EJEMPLO"<<endl;
	cout<<"\t./Ej2TP3 [Nro de Threads]"<<endl;

}

bool is_number(const std::string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void serieFibo(int num)
{
	/*extern int vFibo[];
	/*int fibo1=1;
    int fibo2=1;*/
    long double fibo[num];
    long double res = 0;
    fibo[0]=1;
    fibo[1]=1;
	for(int j = 2; j <= num; j++) 
	{
    	fibo[j] = fibo[j - 2] + fibo[j - 1];
  	}
  	for(int i = 0; i <= num; i++) 
	{
    	res += fibo[i];
  	}
	vFibo[num] = res;
}

void producto(int cantHilos)
{
	long double res = 1;
    int pos = 1;
	for(int j = 0; j < cantHilos; j++) 
	{
		res *= vFibo[j];
  	}
  	
	sumProd[pos] = res;
}

void suma(int cantHilos)
{
    long double res = 0;
    int pos = 0;
	for(int j = 0; j < cantHilos; j++) 
	{
		res += vFibo[j];
  	}

	sumProd[pos] = res;
}

int main(int argc, char const *argv[]){

	/*Creacion de variables*/

	/*Verifico que la cantidad de argumentos sea la correcta*/
	if (argc == 2)
	{
	    if( strcmp(argv[1],"-help") ==  0 ||  strcmp(argv[1],"-h") == 0 ||  strcmp(argv[1],"-?") == 0 )			
			{
				ayuda();
				exit(0);
			}
        
	}
	else
	{
		cout<<"Error en los parametros, ingrese el parametro -help, -h o -? para mas informacion"<<endl;
        cout<<"./Ej2-TP3 -help"<<endl;
		exit(EXIT_FAILURE);
	}

	if(is_number(argv[1]))
	{
		cantThreads = atoi(argv[1]);
	}
	else
	{
		cout<<"La cantidad de hilos debe ser ingresada de forma numerica y mayor a 0."<<endl;
		exit(EXIT_FAILURE);
	}
	

	if ( cantThreads < 0 )
	{
		cout<<"El numero de threads debe ser mayor a 0"<<endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		/*HACER UN FOR QUE VAYA DE 1 HASTA LA CANT DE THREADS*/
		vFibo.resize(cantThreads);
		myThreads.resize(cantThreads);

	    for (int i=0; i<cantThreads; i++){
	       
	        myThreads[i] = thread(serieFibo, i);
	        //new (&myThreads[i]) std::thread(exec, i); //I tried it and it seems to work, but it looks like a bad design or an anti-pattern.
	    }
	    for (int i=0; i<cantThreads; i++){
	        myThreads[i].join();
	    }

		thread t1 = thread(suma, cantThreads);
		thread t2 = thread(producto, cantThreads);
	    
	    t1.join();
	    t2.join();

	    long double res = sumProd[1] - sumProd[0];
	    cout<<"El resultado es: "<<res<<endl;

	}
	
	return 0;
}