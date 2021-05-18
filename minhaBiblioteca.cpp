#include <pthread.h>
#include <iostream>
#include <list>
#include <iterator>
#include <malloc.h>
#include "minhaBiblioteca.h"

using namespace std;

typedef struct Trabalho
{
    int tId;               // ID do Trabalho
    void *(*func)(void *); // Função a ser executada
    void *dta;             // Parâmetros de f
    void *res;             // Retorno de f
} Trabalho;

list<Trabalho> listaTrabalhos, listaResultados;
sem_t plivre, pocupada;
pthread_mutex_t m_Trabalhos, m_Resultados;

static pthread_t *pvs; // Processadores virtuais
int fim;               //Variavel para indicar o fim do programa
int nPvs;              //Variavel que vai conter o numero m de processadores virtuais, para conseguir dar join neles na função finish
int ids = 0;           //Variavel para controlar os IDs dos trabalhos
Trabalho *pegaTrabalho()
{
    Trabalho *trab;

    // if (listaTrabalhos.empty())
    // return NULL;

    sem_wait(&pocupada);
    pthread_mutex_lock(&m_Trabalhos);
    trab = listaTrabalhos.front();
    listaTrabalhos.pop_front();
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&plivre);
    sleep(1);
    return trab;
}

void guardaResultado(Trabalho *trab, void *res)
{
    sem_wait(&plivre);
    pthread_mutex_lock(&m_Resultados);
    trab->res = res;
    listaResultados.push_back(trab);
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&pocupada);
    sleep(1);
}

void *criaPv(void *dta)
{
    void *res;
    Trabalho *trab;

    while (!listaTrabalhos.empty())
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
    pthread_mutex_init(&m, NULL);
    sem_init(&plivre, 0, m);
    sem_init(&pocupada, 0, 0);
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

    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL)
    {
        return 0;
    }
    sem_wait(&plivre);
    pthread_mutex_lock(&m_Trabalhos);
    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_back(trab);
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&pocupada);
    sleep(1);
    return ids;
}
int sync(int tId, void **res)
{
    

    sem_wait(&pocupada);
    pthread_mutex_lock(&m_Resultados);
     list <Trabalho> :: iterator it;
    for(it = listaTrabalhos.begin(); it != listaTrabalhos.end(); ++it){
        
    }
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&plivre);
    sleep(1);
}