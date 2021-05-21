#include <pthread.h>
#include <unistd.h>
#include <list>
#include <malloc.h>
#include <semaphore.h>
#include "minhaBiblioteca.h"
#include <stdio.h>

using namespace std;

typedef struct Trabalho
{
    int tId;               // ID do Trabalho
    void *(*func)(void *); // Função a ser executada
    void *dta;             // Parâmetros de f
    void *res;             // Retorno de f
} Trabalho;

list<Trabalho*> listaTrabalhos, listaResultados;

//sem_t plivre, pocupada, rlivre, rocupada;
// pthread_mutex_t m_Trabalhos, m_Resultados;

pthread_mutex_t m_trab = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m_res = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t var_trab = PTHREAD_COND_INITIALIZER;
pthread_cond_t var_res = PTHREAD_COND_INITIALIZER;

static pthread_t *pvs; // Processadores virtuais
static int fim = 0;    //Variavel para indicar o fim do programa
static int nPvs;       //Variavel que vai conter o numero m de processadores virtuais, para conseguir dar join neles na função finish
static int ids = 0;    //Variavel para controlar os IDs dos trabalhos

Trabalho *pegaTrabalho()
{
    printf("-> PEGA TRABALHO    THREAD: %d\n", (int *)pthread_self());
    Trabalho *trab;

    pthread_mutex_lock(&m_trab);

    while (listaTrabalhos.empty()) pthread_cond_wait(&var_trab, &m_trab);

    printf("-- DENTRO DE PEGA TRABALHO    THREAD: %d\n", (int *)pthread_self());
    trab = listaTrabalhos.front();
    listaTrabalhos.pop_front();
    //printf("        ID = %d       DTA = %d      RES = %d    LIST = %d\n", trab->tId, *(int*)trab->res, *(int*)trab->dta, trab_size);
    printf("        ID = %d       DTA = %d      LIST = %d\n", trab->tId, *(int*)trab->dta, listaTrabalhos.size());
    pthread_mutex_unlock(&m_trab);

    return trab;
}

void guardaResultado(Trabalho* trab, void *res)
{
    printf("-> GUARDA RESULTADO   ID: %d    R = %d    THREAD: %d\n", trab->tId, *(int*)res, (int *)pthread_self());
    pthread_mutex_lock(&m_res);

    printf("-- DENTRO DE GUARDA RESULTADO    THREAD: %d\n", (int *)pthread_self());
    trab->res = res;
    listaResultados.push_back(trab);

    printf("        ID = %d       DTA = %d      RES = %d    LIST = %d\n", trab->tId, *(int*)trab->res, *(int*)trab->dta, listaResultados.size());

    if (!listaResultados.empty()) pthread_cond_signal(&var_res);
    //if (listaResultados.size() > 0) pthread_cond_broadcast(&var_res);
    pthread_mutex_unlock(&m_res);

}

void *criaPv(void *dta)
{
    printf("-> CRIA PV    THREAD: %d\n", (int *)pthread_self());
    void *res;
    Trabalho *trab;

    sleep(1);

    while(!fim){
        /* if (trab_size > 0)
        { */
        trab = pegaTrabalho();

        printf("-> FIBO THREAD: %d    ", (int *)pthread_self());
        res = trab->func(trab->dta);

        guardaResultado(trab, res);
        /* } */
    }

    return NULL;
}

int start(int m)
{
    printf("-> START    THREAD: %d\n", (int *)pthread_self());
    int retorno = 0;

    nPvs = m;

    pvs = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < nPvs; i++)
        retorno = pthread_create(&pvs[i], NULL, criaPv, NULL);

    sleep(1);

    return retorno;
}

void finish()
{
    printf("-> FINISH    THREAD: %d\n", (int *)pthread_self());
    fim = 1;
    
    for (int i = 0; i < nPvs; i++)
        pthread_join(pvs[i], NULL);
}

int spawn(struct Atrib *atrib, void *(*t)(void *), void *dta)
{
    printf("-> SPAWN    THREAD: %d\n", (int *)pthread_self());
    Trabalho *trab;
    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL) return 0;

    pthread_mutex_lock(&m_trab);

    printf("-- DENTRO DE SPAWN    THREAD: %d\n", (int *)pthread_self());
    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_back(trab);
    //printf("        ID = %d       DTA = %d      RES = %d    LIST = %d\n", trab->tId, *(int*)trab->res, *(int*)trab->dta, trab_size);
    printf("        ID = %d       DTA = %d      LIST = %d\n", trab->tId, *(int*)trab->dta, listaTrabalhos.size());
    if (!listaTrabalhos.empty()) pthread_cond_signal(&var_trab);
    //if (listaTrabalhos.size() > 0) pthread_cond_broadcast(&var_trab);
    
    pthread_mutex_unlock(&m_trab);

    return ids;
}

Trabalho* pegaTrabalhoPorId(int tId, list<Trabalho*> *lista) {
    list <Trabalho*> :: iterator it;

    Trabalho* trab;

    //for(it = lista->begin(); it != lista->end(); it++) {
    for(it = listaResultados.begin(); it != listaResultados.end(); it++) {
        // printf("ID -> %d    TRABALHO -> %d\n", (*it)->tId, *(int*)((*it)->dta));
        if ((*it)->tId == tId) {
            trab = *it;
            listaResultados.erase(it);
            //lista->erase(it);
            //res_size = lista->size();
            break;
        }
    }
    
    return trab;
}


int sync(int tId, void **res)
{
    printf("-> SYNC    THREAD: %d\n", (int *)pthread_self());
    Trabalho* trab;

    pthread_mutex_lock(&m_res);
    while (listaResultados.empty()) pthread_cond_wait(&var_res, &m_res);

    printf("-- DENTRO DE SYNC    THREAD: %d\n", (int *)pthread_self());

    while (trab == NULL)
        trab = pegaTrabalhoPorId(tId, &listaResultados);

    res = (void **)(trab->res);
    printf("        ID = %d       DTA = %d      RES = %d    LIST = %d\n", trab->tId, *(int*)trab->res, *(int*)trab->dta, listaResultados.size());

    pthread_mutex_unlock(&m_res);

    return 1;
    /* // CASO 1
    printf("LISTA TRABALHOS\n");
    trab = pegaTrabalhoPorId(tId, listaTrabalhos);
    if (trab == NULL) {
        // CASO 2
        printf("LISTA RESULTADOS\n");
        trab = pegaTrabalhoPorId(tId, listaResultados);
        if (trab == NULL) {
            sleep(1);
            // CASO 3
            trab = pegaTrabalhoPorId(tId, listaResultados);
            if (trab == NULL)
                return 0;
            else
                res = (void **)(trab->res);

        } else {
            res = (void **)(trab->res);

        }
    } else {
        res = (void **)(trab->func(trab->dta));

    } */

}