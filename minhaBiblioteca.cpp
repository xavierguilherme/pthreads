#include <pthread.h>
#include <malloc.h>
#include "minhaBiblioteca.h"

typedef struct Trabalho
{
    int tId;               // ID do Trabalho
    void *(*func)(void *); // Função a ser executada
    void *dta;             // Parâmetros de f
    void *res;             // Retorno de f
    struct Trabalho *prev, *next;
} Trabalho;

typedef struct Pos
{
    Trabalho *head;
    Trabalho *tail;
} Pos;

Trabalho *listaTrabalhos, *listaResultados;
Pos *posTrabalhos, *posResultados; // Posições nas listas

static pthread_t *pvs; // Processadores virtuais
int fim;               //Variavel para indicar o fim do programa
int nPvs;              //Variavel que vai conter o numero m de processadores virtuais, para conseguir dar join neles na função finish
int ids = 0;           //Variavel para controlar os IDs dos trabalhos

Trabalho *pegaTrabalho()
{
    Trabalho *trab = malloc(sizeof(Trabalho));
    if (listaTrabalhos == NULL)
        return NULL;

    trab = posTrabalhos->head;
    posTrabalhos->head = posTrabalhos->head->next;
    if (posTrabalhos->head == NULL)
        return trab;

    posTrabalhos->head->prev = NULL;
    trab->next = NULL;
    return trab;
}

void guardaResultado(Trabalho *trab, void *res)
{
    trab->res = res;
    if (listaResultados == NULL)
    {
        trab = malloc(sizeof(Trabalho));
        posResultados->head = trab;
        posResultados->head->prev = NULL;
        posResultados->head->next = NULL;
    }
    else
    {
        posResultados->tail->next = trab;
        trab->prev = posResultados->tail;
        posResultados->tail = trab;
    }
}

void *criaPv(void *dta)
{
    void *res;
    Trabalho *trab;

    while (listaTrabalhos != NULL)
    {
        trab = pegaTrabalho();
        res = trab->func(trab->dta);
        guardaResultado(trab, res);
    }

    return NULL;
}

int start(int m)
{
    int retorno = 0;

    fim = 0;
    nPvs = m;
    pvs = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < nPvs; i++)
        retorno = pthread_create(&pvs[i], NULL, criaPv, NULL);

    return retorno;
}

void finish()
{
    fim = 1;

    for (int i = 0; i < nPvs; i++)
        pthread_join(pvs[i], NULL);
}
int spawn(struct Atrib *atrib, void *(*t)(void *), void *dta)
{
    Trabalho *trab;

    trab = malloc(sizeof(Trabalho));
    if (trab == NULL)
    {
        return 0;
    }
    else
    {
        ids++;
        trab->tId = ids;
        trab->func = t;
        trab->dta = dta;

        if (listaTrabalhos == NULL)
        {
            posTrabalhos->head = trab;
            posTrabalhos->head->prev = NULL;
            posTrabalhos->head->next = NULL;
        }
        else
        {
            posTrabalhos->tail->next = trab;
            trab->prev = posTrabalhos->tail;
            posTrabalhos->tail = trab;
        }

        return ids;
    }
}