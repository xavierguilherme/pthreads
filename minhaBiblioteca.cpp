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

list<Trabalho *> listaTrabalhos, listaResultados;
pthread_mutex_t m_trab = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m_res = PTHREAD_MUTEX_INITIALIZER;

static pthread_t *pvs;
static bool fim = false;
int nPvs;
static int ids = 0;

void *criaPv(void *dta)
{
    void *res;
    Trabalho *trab;

    sleep(3);

    while (!fim)
    {
        if (!listaTrabalhos.empty())
        {

            Trabalho *trab;

            pthread_mutex_lock(&m_trab);
            trab = listaTrabalhos.front();
            listaTrabalhos.pop_front();
            pthread_mutex_unlock(&m_trab);

            sleep(1);

            printf("\n(%d)(FIBO) ", pthread_self());
            res = trab->func(trab->dta);

            pthread_mutex_lock(&m_res);
            trab->res = res;
            listaResultados.push_back(trab);
            pthread_mutex_unlock(&m_res);
            sleep(1);
        }
    }
    return NULL;
}

int start(int m)
{
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
    fim = true;

    for (int i = 0; i < nPvs; i++)
        pthread_join(pvs[i], NULL);

    pthread_mutex_destroy(&m_trab);
    pthread_mutex_destroy(&m_res);
}

int spawn(struct Atrib *atrib, void *(*t)(void *), void *dta)
{
    Trabalho *trab;
    trab = (Trabalho *)malloc(sizeof(Trabalho));
    if (trab == NULL)
        return 0;

    pthread_mutex_lock(&m_trab);

    ids++;
    trab->tId = ids;
    trab->func = t;
    trab->dta = dta;
    listaTrabalhos.push_front(trab);

    pthread_mutex_unlock(&m_trab);

    return ids;
}

int sync(int tId, void **res)
{
    list<Trabalho *>::iterator it;
    Trabalho t;
    bool found = false;
    int idx;

    pthread_mutex_lock(&m_res);
    for (it = listaResultados.begin(); it != listaResultados.end(); it++)
    {
        if ((*it)->tId == tId)
        {
            *res = (void **)((*it)->res);
            listaTrabalhos.erase(it);
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&m_res);
    if (found)
        return 1;

    found = false;
    pthread_mutex_lock(&m_trab);
    idx = -1;
    for (it = listaTrabalhos.begin(); it != listaTrabalhos.end(); it++)
    {
        idx++;
        if ((*it)->tId == tId)
        {
            t = *(*it);
            listaTrabalhos.erase(it);
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&m_trab);
    if (found)
    {
        *res = t.func(t.dta);
        return 1;
    }
    found = false;
    while (!found)
    {
        pthread_mutex_lock(&m_res);
        for (it = listaResultados.begin(); it != listaResultados.end(); it++)
        {
            if ((*it)->tId == tId)
            {
                *res = (void **)((*it)->res);
                listaTrabalhos.erase(it);
                found = true;
                break;
            }
        }
    }
    pthread_mutex_unlock(&m_res);
    if (found)
        return 1;

    sleep(1);
    return 0;
}