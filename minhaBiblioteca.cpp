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
sem_t plivre, pocupada, rlivre, rocupada;
pthread_mutex_t m_Trabalhos, m_Resultados;

static pthread_t *pvs; // Processadores virtuais
int fim = 0;               //Variavel para indicar o fim do programa
int nPvs;              //Variavel que vai conter o numero m de processadores virtuais, para conseguir dar join neles na função finish
int ids = 0;           //Variavel para controlar os IDs dos trabalhos

Trabalho *pegaTrabalho()
{
    printf("PEGA TRABALHO\n");
    Trabalho *trab;

     //if (listaTrabalhos.empty())
     //return NULL;

    sem_wait(&pocupada);
    pthread_mutex_lock(&m_Trabalhos);
    printf("-- DENTRO DE PEGA TRABALHO\n");
    trab = listaTrabalhos.front();
    printf("ID DE PEGA TRABALHO = %d\n", trab->tId);
    listaTrabalhos.pop_front();
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&plivre);
    sleep(1);

    return trab;
}

void guardaResultado(Trabalho* trab, void *res)
{
    printf("GUARDA RESULTADO    %d\n", *(int*)res);
    sem_wait(&rlivre);
    pthread_mutex_lock(&m_Resultados);
    printf("-- DENTRO DE GUARDA RESULTADO\n");
    trab->res = res;
    listaResultados.push_back(trab);
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&rocupada);
    sleep(1);
}

void *criaPv(void *dta)
{
    printf("CRIA PV\n");
    void *res;
    Trabalho *trab;
    while(!fim){
        if (!listaTrabalhos.empty())
        {
            trab = pegaTrabalho();
            printf("            DTA = %d\n", *(int*)trab->dta);
            res = trab->func(trab->dta);
            guardaResultado(trab, res);
        }
    }
    return NULL;
}

int start(int m)
{
    printf("START\n");
    int retorno = 0;
    pthread_mutex_init(&m_Trabalhos, NULL);
    pthread_mutex_init(&m_Resultados, NULL);
    sem_init(&plivre, 0, m);
    sem_init(&pocupada, 0, 0);
    sem_init(&rlivre, 0, m);
    sem_init(&rocupada, 0, 0);

    nPvs = m;

    pvs = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < nPvs; i++)
        retorno = pthread_create(&pvs[i], NULL, criaPv, NULL);

    sleep(1);

    return retorno;
}

void finish()
{
    printf("FINISH\n");
    fim = 1;

    pthread_mutex_destroy(&m_Trabalhos);
    pthread_mutex_destroy(&m_Resultados);
    sem_destroy(&plivre);
    sem_destroy(&pocupada);
    sem_destroy(&rlivre);
    sem_destroy(&rocupada);
    
    for (int i = 0; i < nPvs; i++)
        pthread_join(pvs[i], NULL);
}

int spawn(struct Atrib *atrib, void *(*t)(void *), void *dta)
{
    printf("SPAWN   DTA: %d\n", *(int*)dta);
    Trabalho *trab;
    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL){
        return 0;
    }

    /* int value;
    sem_getvalue(&plivre, &value);
    printf("SEM: %d\n", value); */
    

    sem_wait(&plivre);
    pthread_mutex_lock(&m_Trabalhos);
    printf("-- DENTRO DE SPAWN\n");
    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_back(trab);
    printf("ID TRAB SPAWNADO = %d\n", trab->tId);
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&pocupada);
    
    return ids;
}

Trabalho* pegaTrabalhoPorId(int tId, list<Trabalho*> lista) {
    list <Trabalho*> :: iterator it;

    printf("TAMANHO DA LISTA = %d\n", lista.size());

    Trabalho* trab;

    for(it = lista.begin(); it != lista.end(); it++) {
        printf("ID -> %d    TRABALHO -> %d\n", (*it)->tId, *(int*)((*it)->dta));
        if ((*it)->tId == tId) {
            trab = *it;
            lista.erase(it);
            break;
        }
    }

    return trab;
}

int sync(int tId, void **res)
{
    printf("SYNC    ID: %d\n", tId);
    Trabalho* trab;

    sem_wait(&rocupada);
    pthread_mutex_lock(&m_Resultados);
    printf("-- DENTRO DE SYNC\n");
    // CASO 1
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
    }
    printf("SYNC    RES: %d\n", *(int*)res);
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&rlivre);
    sleep(1);

    return 1;
}