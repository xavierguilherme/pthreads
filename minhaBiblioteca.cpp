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
static int oct, ocr, livt, livr;

void *criaPv(void *dta)
{
    printf("(%d)(CRIA PV)\n", pthread_self());
    void *res;
    Trabalho *trab;

    sleep(3);

    while(!fim){
        
        if (!listaTrabalhos.empty()) {

            Trabalho *trab;

            // PEGA O TRABALHO
            sem_getvalue(&tocupada, &oct);
            sem_getvalue(&tlivre, &livt);
            printf("\n(%d)(PEGA) TOCUP = %d, TLIV = %d\n", pthread_self(), oct, livt);
            sem_wait(&tocupada);
            sem_getvalue(&tocupada, &oct);
            sem_getvalue(&tlivre, &livt);
            printf("(%d)(DENTRO PEGA) TOCUP = %d, TLIV = %d\n", pthread_self(), oct, livt);
            pthread_mutex_lock(&m_trab);
            trab = listaTrabalhos.front();
            listaTrabalhos.pop_front();   
            pthread_mutex_unlock(&m_trab);
            sem_post(&tlivre);
            sem_getvalue(&tocupada, &oct);
            sem_getvalue(&tlivre, &livt);
            printf("(%d)(DEPOIS PEGA) ID = %d DTA = %d LIST = %d TOCUP = %d, TLIV = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, listaTrabalhos.size(), oct, livt);

            sleep(1);

            // CHAMA A FUNÇÃO
            printf("\n(%d)(FIBO) ", pthread_self());
            res = trab->func(trab->dta);

            // GUARDA O RESULTADO
            sem_getvalue(&rocupada, &ocr);
            sem_getvalue(&rlivre, &livr);
            printf("\n(%d)(GUARDA) ID: %d RES = %d ROCUP = %d, RLIV = %d\n", pthread_self(), trab->tId, *(int*)res, ocr, livr);
            sem_wait(&rlivre);
            sem_getvalue(&rocupada, &ocr);
            sem_getvalue(&rlivre, &livr);
            printf("(%d)(DENTRO GUARDA) ROCUP = %d, RLIV = %d\n", pthread_self(), ocr, livr);
            pthread_mutex_lock(&m_res);
            trab->res = res;
            listaResultados.push_back(trab);
            pthread_mutex_unlock(&m_res);
            sem_post(&rocupada);
            sem_getvalue(&rocupada, &ocr);
            sem_getvalue(&rlivre, &livr);
            printf("(%d)(DEPOIS GUARDA) ID = %d DTA = %d RES = %d LIST = %d ROCUP = %d, RLIV = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, *(int*)trab->res, listaResultados.size(), ocr, livr);
            
            sleep(1);
        }

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
    sem_getvalue(&tocupada, &oct);
    sem_getvalue(&tlivre, &livt);
    printf("\n(%d)SPAWN TOCUP = %d, TLIV = %d\n", pthread_self(), oct, livt);
    Trabalho *trab;
    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL) return 0;

    
    sem_wait(&tlivre);
    sem_getvalue(&tocupada, &oct);
    sem_getvalue(&tlivre, &livt);
    printf("(%d)(DENTRO SPAWN) TOCUP = %d, TLIV = %d\n", pthread_self(), oct, livt);
    pthread_mutex_lock(&m_trab);

    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_front(trab);
    
    pthread_mutex_unlock(&m_trab);
    sem_post(&tocupada);
    sem_getvalue(&tocupada, &oct);
    sem_getvalue(&tlivre, &livt);
    printf("(%d)(DEPOIS SPAWN) ID = %d DTA = %d LIST = %d TOCUP = %d, TLIV = %d\n", pthread_self(), trab->tId, *(int*)trab->dta, listaTrabalhos.size(), oct, livt);

    sleep(1);

    return ids;
}

/* Trabalho* pegaTrabalhoPorId(int tId, list<Trabalho*> *lista) {
    list <Trabalho*>::iterator it;

    Trabalho* trab;

    //for(it = lista->begin(); it != lista->end(); it++) {
    for(it = listaResultados.begin(); it != listaResultados.end(); ++it) {
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
} */

int sync(int tId, void **res)
{
    list <Trabalho*>::iterator it;
    bool found = false;
    int idx;

    while(!found) {
        idx = -1;
        for(it = listaResultados.begin(); it != listaResultados.end(); it++) {
            idx++;
            if ((*it)->tId == tId) {
                found = true;
                break;
            }
        }
    }

    sem_getvalue(&rocupada, &ocr);
    sem_getvalue(&rlivre, &livr);
    printf("\n(%d)(SYNC) ID = %d ROCUP = %d, RLIV = %d\n", pthread_self(), tId, ocr, livr);

    sem_wait(&rocupada);

    sem_getvalue(&rocupada, &ocr);
    sem_getvalue(&rlivre, &livr);
    printf("(%d)(DENTRO SYNC) ID = %d ROCUP = %d, RLIV = %d\n", pthread_self(), tId, ocr, livr);

    pthread_mutex_lock(&m_res);

    it = listaResultados.begin();
    advance(it, idx);
    res = (void **)((int*)(*it)->res);
    listaResultados.erase(it);

    /* while (trab == NULL)
        trab = pegaTrabalhoPorId(tId, &listaResultados); */

    pthread_mutex_unlock(&m_res);
    sem_post(&rlivre);

    sem_getvalue(&rocupada, &ocr);
    sem_getvalue(&rlivre, &livr);
    printf("(%d)(DEPOIS SYNC) ID = %d RES = %d LIST = %d ROCUP = %d, RLIV = %d\n", pthread_self(), tId, *(int*)res, listaResultados.size(), ocr, livr);
    
    sleep(1);

    return 1;
}