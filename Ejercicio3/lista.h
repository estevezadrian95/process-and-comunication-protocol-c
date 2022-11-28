#ifndef LISTA_H_INCLUDED
#define LISTA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

typedef char t_dato[256];

typedef struct s_nodo* t_lista;

typedef struct s_nodo
{
    t_dato dato;
    struct s_nodo* psig;
}t_nodo;


void lista_crear (t_lista *pl);
void lista_recorrer (t_lista* pl,void* param,void(*accion)(t_dato*,void*));
int lista_vacia(t_lista *pl);
int lista_poner_ult (t_lista *pl, t_dato *pd);
void lista_eliminar(t_lista* pl);

#endif // LISTA_H_INCLUDED
