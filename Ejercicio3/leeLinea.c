#include "leeLinea.h"
#include "lista.h"
#include <string.h>

#define BUF_LEN 256
void concatena_string(t_dato *pd, void* stringAConcatenar);
char* resultado = NULL;

char* leeLinea(FILE* fp)
{
    resultado = NULL;
    char buffer[BUF_LEN];
    int lineEnded = 0;
    int proceso = 0;
    t_lista lista;

    lista_crear(&lista);
    while (!lineEnded && fgets(buffer, BUF_LEN, fp) != NULL) {
        lineEnded = strchr(buffer, '\n') != NULL;
        // printf("termino linea %d \n", lineEnded);
        // printf("buffer %s \n", buffer);
        proceso = 1;
        if (!lineEnded) {
            //printf("No termino la linea !\n");
            lista_poner_ult(&lista, (t_dato*) buffer);
            int terminoFile = fgets(buffer, BUF_LEN, fp) != NULL;
            if(!terminoFile)
            {
                //printf("Porque termino la file !\n");
                break;
            } 
            // la linea sigue
            // acá hay que hacer la tramoya de la lista...
        }
        if ((strlen(buffer)>0) && (buffer[strlen (buffer) - 1] == '\n'))
            buffer[strlen (buffer) - 1] = '\0'; 
        lista_poner_ult(&lista, (t_dato*) buffer);
    }
    //lista_recorrer(&lista, &resultado, construye_resultado);
    lista_recorrer(&lista, resultado, concatena_string);
    lista_eliminar(&lista);
    return proceso ? resultado : NULL;
}

char* concat(char *s1, char *s2)
{
    int len1 = 0;
    int len2 = 0;
    //printf("%s s1 %s s2\n", s1, s2);
    if (s1 != NULL) {
        len1 = strlen(s1);
    }
    if (s2 != NULL) {
        len2 = strlen(s2);
    }
    if (len1 == 0 && len2 == 0) {
        return NULL;
    }
    //printf("%d lens1 %d lens2\n", len1, len2);
    char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
    memset(result, '\0', len1 + len2 + 1);
    // in real code you would check for errors in malloc here
    if (s1 != NULL) {
        strcpy(result, s1);
    }
    if (s2 != NULL) {
        strcat(result, s2);
    }
    return result;
}

void concatena_string(t_dato *pd, void* pstringAConcatenar)
{
    char* stringAConcatenar = (char*) ((void**) pstringAConcatenar);
    
    resultado = concat((char*) stringAConcatenar, (char*) pd);
}
