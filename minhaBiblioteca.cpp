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

sem_t tlivre, tocupada, rlivre, rocupada;

pthread_mutex_t m_trab = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m_res = PTHREAD_MUTEX_INITIALIZER;

static pthread_t *pvs; // Processadores virtuais
static bool fim = false;    //Variavel para indicar o fim do programa
static int nPvs;       //Variavel que vai conter o numero m de processadores virtuais, para conseguir dar join neles na função finish
static int ids = 0;    //Variavel para controlar os IDs dos trabalhos

void *criaPv(void *dta)
{
    printf("(%d)(CRIA PV)\n", pthread_self());
    void *res;
    Trabalho *trab;

    sleep(1);

    while(!fim){

        Trabalho *trab;

        // PEGA O TRABALHO
        printf("(%d)(PEGA)\n", pthread_self());

        sem_wait(&tocupada);
        printf("(%d)(DENTRO PEGA)\n", pthread_self());
        pthread_mutex_lock(&m_trab);

        trab = listaTrabalhos.front();
        listaTrabalhos.pop_front();

        printf("(%d)(DEPOIS PEGA) ID = %d DTA = %d LIST = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, listaTrabalhos.size());
        
        pthread_mutex_unlock(&m_trab);
        sem_post(&tlivre);

        sleep(1);

        // CHAMA A FUNÇÃO
        printf("(%d)(FIBO) ", pthread_self());
        res = trab->func(trab->dta);

        // GUARDA O RESULTADO

        printf("(%d)(GUARDA) ID: %d RES = %d\n", pthread_self(), trab->tId, *(int*)res);
        sem_wait(&rlivre);
        printf("(%d)(DENTRO GUARDA)\n", pthread_self());
        pthread_mutex_lock(&m_res);

        trab->res = res;
        listaResultados.push_back(trab);

        printf("(%d)(DEPOIS GUARDA) ID = %d DTA = %d RES = %d LIST = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, *(int*)trab->res, listaResultados.size());

        pthread_mutex_unlock(&m_res);
        sem_post(&rocupada);
        
        sleep(1);

    }

    return NULL;
}

int start(int m)
{
    printf("(%d)(START)\n", pthread_self());
    int retorno = 0;

    nPvs = m;

    sem_init(&tlivre, 0, nPvs);
    sem_init(&tocupada, 0, 0);

    sem_init(&rlivre, 0, nPvs);
    sem_init(&rocupada, 0, 0);

    pvs = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < nPvs; i++)
        retorno = pthread_create(&pvs[i], NULL, criaPv, NULL);

    sleep(1);

    return retorno;
}

void finish()
{
    printf("(%d)(FINISH)\n", pthread_self());
    fim = true;

    for (int i = 0; i < nPvs; i++) 
        pthread_join(pvs[i], NULL);
    
    pthread_mutex_destroy(&m_trab);
    pthread_mutex_destroy(&m_res);
}

int spawn(struct Atrib *atrib, void *(*t)(void *), void *dta)
{
    printf("(%d)SPAWN\n", pthread_self());
    Trabalho *trab;
    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL) return 0;

    sem_wait(&tlivre);
    printf("(%d)(DENTRO SPAWN)\n", pthread_self());
    pthread_mutex_lock(&m_trab);

    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_back(trab);

    printf("(%d)(DEPOIS SPAWN) ID = %d DTA = %d LIST = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, listaTrabalhos.size());
    
    pthread_mutex_unlock(&m_trab);
    sem_post(&tocupada);

    sleep(1);

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
    printf("(%d)(SYNC) ID = %d\n", pthread_self(), tId);
    Trabalho* trab;

    sem_wait(&rocupada);
    printf("(%d)(DENTRO SYNC) ID = %d\n", pthread_self(), tId);
    pthread_mutex_lock(&m_res);

    while (trab == NULL)
        trab = pegaTrabalhoPorId(tId, &listaResultados);

    res = (void **)(trab->res);
    printf("(%d)(DEPOIS SYNC) ID = %d DTA = %d RES = %d LIST = %d\n", pthread_self(), trab->tId, *(int*)trab->res, *(int*)trab->dta, listaResultados.size());

    pthread_mutex_unlock(&m_res);
    sem_post(&rlivre);

    sleep(1);

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