#include <pthread.h>
#include <unistd.h>
#include <list>
#include <malloc.h>
#include <semaphore.h>
#include <stdio.h>
#include "minhaBiblioteca.h"

using namespace std;

typedef struct Trabalho
{
    int tId;               // ID do Trabalho
    void *(*func)(void *); // Função a ser executada
    void *dta;             // Parâmetros de f
    void *res;             // Retorno de f
} Trabalho;

list<Trabalho *> listaTrabalhos, listaResultados;
pthread_mutex_t m_trab = PTHREAD_MUTEX_INITIALIZER; // Mutex da Lista de Trabalhos prontos
pthread_mutex_t m_res = PTHREAD_MUTEX_INITIALIZER;  // Mutex da Lista de Trabalhos finalizados.

static pthread_t *pvs;
static bool fim = false; // Variável que indica o fim do programa
static int nPvs; // Mantém a quantidade de processadores virtuais instanciados
static int ids = 0; // Mantém o ID da próxima tarefa executada

/*
Esta função busca uma tarefa na lista de trabalhos prontos,
executa e guarda essa tarefa na lista de trabalhos terminados. 
*/
void *criaPv(void *dta)
{
    void *res;
    Trabalho *trab;

    sleep(3);

    while (!fim) {
        if (!listaTrabalhos.empty()) {
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

/*
Esta primitiva lança o núcleo de execução, instanciando m processadores virtuais, 
indicados pelo parâmetro m. O retorno 0 (zero) indica falha na instanciação dos 
processadores virtuais. Um valor maior que 0 indica criação bem sucedida.
*/
int start(int m)
{
    int retorno = 0;

    nPvs = m;
    pvs = (pthread_t *)malloc(m * sizeof(pthread_t));

    for (int i = 0; i < nPvs; i++)
        retorno |= pthread_create(&pvs[i], NULL, criaPv, NULL);

    sleep(1);

    return retorno;
}

/*
Esta primitiva é bloqueante, retornando após todos os processadores virtuais terem finalizado.
*/
void finish()
{
    fim = true;

    for (int i = 0; i < nPvs; i++)
        pthread_join(pvs[i], NULL);

    pthread_mutex_destroy(&m_trab);
    pthread_mutex_destroy(&m_res);
}

/*
A primitiva spawn lança a execução da tarefa descrita no ponteiro para função 
indicada pelo parâmetro t. O parâmetro para a função *t é descrito no parâmetro dta. 
O parâmetro struct Atrib* atrib descreve os atributos a serem considerados no escalonamento 
da tarefa. A função retorna 0 (zero) em caso de falha na criação da tarefa ou um valor inteiro 
positivo maior que 0, considerado o identificador único da tarefa no programa. Caso NULL seja 
passado como endereço para atrib, devem ser considerados os valores default para os atributos.
*/
int spawn(Atrib *atrib, void *(*t)(void *), void *dta)
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

/*
A primitiva sync é bloqueante: a tarefa que invoca a primitiva sync 
informa qual tarefa, identificada no parâmetro tId, que deve ser sincronizada. 
O retorno da primitiva é 0 (zero) em caso de falha ou 1 (um), em caso de sincronização 
bem sucedida. O parâmetro res contém, como saída, o endereço de memória que contém os 
resultados de saída.
*/
int sync(int tId, void **res)
{
    list<Trabalho *>::iterator it;
    Trabalho t;
    bool found = false;
    int idx;

    /* 
    Caso UM: a tarefa está na lista de tarefas prontas. 
    Neste caso, a execução da primitiva sync deve retirar 
    a tarefa da lista de tarefas prontas e executá-la, retornando 
    diretamente o resultado, sem a necessidade de incluir a tarefa 
    concluída na lista de tarefas finalizada
    */
    pthread_mutex_lock(&m_trab);
    idx = -1;
    for (it = listaTrabalhos.begin(); it != listaTrabalhos.end(); it++) {
        idx++;
        if ((*it)->tId == tId) {
            t = *(*it);
            listaTrabalhos.erase(it);
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&m_trab);

    if (found) {
        *res = t.func(t.dta);
        return 1;
    }

    /*
    Caso DOIS: a tarefa está na lista de tarefas terminadas. 
    Neste caso, basta retirar a tarefa da lista de tarefas 
    terminadas e recuperar o dado de retorno.
    */
    pthread_mutex_lock(&m_res);
    for (it = listaResultados.begin(); it != listaResultados.end(); it++) {
        if ((*it)->tId == tId) {
            *res = (void **)((*it)->res);
            listaTrabalhos.erase(it);
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(&m_res);

    if (found)
        return 1;
    
    /*
    Caso TRÊS: a tarefa está em execução (não está em nenhuma das listas). 
    Neste caso, a tarefa está sendo executada por um outro pv e a implementação 
    deve solucionar o problema. O pv entra em estado de sleep durante um período 
    de tempo e volta a procurar a tarefa na lista de tarefas finalizadas, apostando 
    que o tempo de espera é curto.
    */
    while (!found) {
        pthread_mutex_lock(&m_res);
        for (it = listaResultados.begin(); it != listaResultados.end(); it++) {
            if ((*it)->tId == tId) {
                *res = (void **)((*it)->res);
                listaTrabalhos.erase(it);
                found = true;
                break;
            }
        }
        pthread_mutex_unlock(&m_res);
    }

    if (found)
        return 1;

    sleep(1);

    return 0;
}