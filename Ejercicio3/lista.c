#include "lista.h"
#include <string.h>

int lista_sacar_pri (t_lista *pl,t_dato *pd);
int lista_poner_primero (t_lista *pl, t_dato *pd);


int lista_poner_primero (t_lista *pl, t_dato *pd)
{
    //printf("%lu size of tdato %lu sizeof del dato\n",sizeof(t_dato), strlen((char*)pd));
   t_nodo *nue = (t_nodo*)malloc(sizeof(t_nodo));
   
    if(!nue)
        return 0;
    strcpy(nue->dato, (char*) pd);
    nue->psig = *pl;
    *pl=nue;
    return 1;
}

void lista_crear (t_lista *pl)
{
    *pl = 0;
}

int lista_poner_ult (t_lista *pl, t_dato *pd)
{
    //printf("dato %s\n", (char*) pd);
    while(*pl)
        pl = &(*pl)->psig;
    return lista_poner_primero(pl,pd);
}

int lista_vacia(t_lista *pl)
{
    if(!pl)
        return 1;
    else return 0;
}

int lista_sacar_pri (t_lista *pl, t_dato *pd)
{
    t_nodo* dom = *pl;
    if (!pl)
        return 0;
    strcpy((char*) pd, dom->dato);
    *pl = dom->psig;
    free(dom);
    return 1;
}


void lista_recorrer(t_lista* pl, void* param, void(*accion)(t_dato*,void*))
{
    //printf("empezó a recorrer la lista en 0x%lX, con param 0x%lX y acción = 0x%lX\n", 
                        // (unsigned long) pl, (unsigned long) param, (unsigned long) &accion);
    while(*pl)
    {
        accion(&(*pl)->dato, param);
        pl=&(*pl)->psig;
    }
}
void lista_eliminar(t_lista* pl)
{
    t_nodo* dom;
    while(*pl)
    {
        dom = *pl;
        *pl = dom->psig;
        free(dom);
    }
}

