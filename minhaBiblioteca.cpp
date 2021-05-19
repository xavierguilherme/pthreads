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

     //if (listaTrabalhos.empty())
     //return NULL;

    sem_wait(&pocupada);
    pthread_mutex_lock(&m_Trabalhos);
    trab = &listaTrabalhos.front();
    printf("%d ", *(int*)trab->dta);
    //listaTrabalhos.pop_front();
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&plivre);
    //sleep(1);
    
    return trab;
}

void guardaResultado(Trabalho* trab, void *res)
{

    sem_wait(&plivre);
    pthread_mutex_lock(&m_Resultados);
    trab->res = res;
    listaResultados.push_back(*trab);
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&pocupada);
    sleep(1);
}

void *criaPv(void *dta)
{
    void *res;
    Trabalho *trab;
    while(!fim){
    if (!listaTrabalhos.empty())
    {
        trab = pegaTrabalho();
        res = trab->func(trab->dta); 
        guardaResultado(trab, res);
    }
        }
    return NULL;
}

int start(int m)
{
    int retorno = 0;
    pthread_mutex_init(&m_Trabalhos, NULL);
    pthread_mutex_init(&m_Resultados, NULL);
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
    listaTrabalhos.push_back(*trab);
    pthread_mutex_unlock(&m_Trabalhos);
    sem_post(&pocupada);
    
    return ids;
}

int sync(int tId, void **res)
{

    int flag = 0;

    sem_wait(&pocupada);
    pthread_mutex_lock(&m_Resultados);
    list <Trabalho> :: iterator it;
    // Caso 1
    /*
     * a tarefa está na lista de tarefas prontas. Neste caso, a execução da primitiva
     * sync deve retirar a tarefa da lista de tarefas prontas e executá-la,
     * retornando diretamente o resultado, sem a necessidade de incluir a tarefa concluída na lista de tarefas finalizada.
     */
    for(it = listaTrabalhos.begin(); it != listaTrabalhos.end(); ++it) {
        if (it->tId == tId) {
            res = (void **)(it->func(it->dta));
            listaTrabalhos.erase(it);
            flag = 1;
            break;
        }
    }

    // Caso 2
    /*
     * a tarefa está na lista de tarefas terminadas. Neste caso,
     * basta retirar a tarefa da lista de tarefas terminadas e recuperar o dado de retorno.
     */
    for(it = listaResultados.begin(); it != listaResultados.end(); ++it) {
        if (it->tId == tId) {
            res = (void **)(it->res);
            listaResultados.erase(it);
            flag = 1;
            break;
        }
    }

    // Caso 3
    /*
     * a tarefa está em execução (não está em nenhuma das listas). Neste caso, a tarefa está sendo executada por um outro pv
     * e a implementação deve solucionar o problema. Possíveis soluções: o pv entra em estado de sleep durante um período de
     * tempo e volta a procurar a tarefa na lista de tarefas finalizadas, apostando que o tempo de espera é curto; o pv retira
     * uma nova tarefa da lista de prontos e inicia sua execução, na expectativa de que quando esta nova tarefa terminar,
     * a tarefa a ser sincronizada terá terminado.
     */
    if (!flag) {
        for(int i = 0; i < 10; i++) {
            sleep(1);
            for (it = listaResultados.begin(); it != listaResultados.end(); ++it) {
                if (it->tId == tId) {
                    res = (void **) (it->res);
                    listaResultados.erase(it);
                    flag = 1;
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&m_Resultados);
    sem_post(&plivre);
    sleep(1);

    return flag;
}