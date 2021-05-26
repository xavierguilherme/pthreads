# Implementação com Pthreads
## Trabalho da disciplina de Sistemas Operacionais
#
#### Objetivo
###### Construir o suporte de execução implementando o modelo n x m de threads para uma linguagem concorrente baseada na criação de tarefas. Este suporte foi implementado na forma de uma biblioteca, utilizando C++ e a biblioteca de threads Pthreads.
###### Neste modelo, n tarefas são executadas sobre m processadores virtuais (pvs). As tarefas são as atividades concorrentes criadas pelo programa (com a diretiva spawn). Os pvs são threads responsáveis por executar as tarefas geradas pelo programa. Os pvs compartilham, entre si, duas listas de tarefas: uma de tarefas prontas para executar e outra de tarefas finalizadas. Os exemplos são calculados de forma recursiva concorrente.   
#
#### Integrantes
- Alejandro Pereira (adspereira@inf.ufpel.edu.br)
- Guilherme Xavier (gdsxavier@inf.ufpel.edu.br)

#### Exemplos implementados
- Fibonacci(n)
- Fatorial(n)
- Soma(n)
- Pow(n, e)

### Passos para execução do programa
`$ mingw32-make -f Makefile`\  
`$ main`