#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include "minhaBiblioteca.h"

/*
Encontra o valor de Fibonacci(n) recursivamente com pthreads.
*/
void *fibo(void *dta)
{
    int *n = (int *)dta;
    int *n1, *n2, *r1, *r2, t1, t2;
    int *r = (int *)malloc(sizeof(int));
    Atrib a1, a2;
    if (*n <= 2) *r = 1;
    else
    {
        n1 = (int *)malloc(sizeof(int));
        *n1 = (*n) - 1;
        a1.p = 0;
        a1.c = *n1;
        t1 = spawn(&a1, fibo, (void *)n1);
        n2 = (int *)malloc(sizeof(int));
        *n2 = (*n) - 2;
        a2.p = 0;
        a1.c = *n2;
        t2 = spawn(&a2, fibo, (void *)n2);
        r1 = (int *)malloc(sizeof(int));
        r2 = (int *)malloc(sizeof(int));
        sync(t1, (void **)&r1);
        sync(t2, (void **)&r2);
        *r = *r1 + *r2;
        free(r1);
        free(r2);
        free(n1);
        free(n2);
    }
    return r;
}

/*
Calcula o fatorial de n recursivamente com pthreads.
*/
void *fat(void *dta)
{
    int *n = (int *)dta;
    int *n1, *r1, t1;
    int *r = (int *)malloc(sizeof(int));
    Atrib a1;
    if (*n <= 1) *r = 1;
    else
    {
        n1 = (int *)malloc(sizeof(int));
        *n1 = (*n) - 1;
        a1.p = 0;
        a1.c = *n1;
        t1 = spawn(&a1, fat, (void *)n1);
        r1 = (int *)malloc(sizeof(int));
        sync(t1, (void **)&r1);
        *r = (*n) * (*r1);
        free(r1);
        free(n1);
    }
    return r;
}

/*
Calcula a soma de um número.
Exemplo: soma(3) = 3 + 2 + 1
*/
void *soma(void *dta)
{
    int *n = (int *)dta;
    int *n1, *r1, t1;
    int *r = (int *)malloc(sizeof(int));
    Atrib a1;
    if (*n == 1) *r = 1;
    else
    {
        n1 = (int *)malloc(sizeof(int));
        *n1 = (*n) - 1;
        a1.p = 0;
        a1.c = *n1;
        t1 = spawn(&a1, soma, (void *)n1);
        r1 = (int *)malloc(sizeof(int));
        sync(t1, (void **)&r1);
        *r = (*n) + (*r1);
        free(r1);
        free(n1);
    }
    return r;
}

/*
Calcula a potência de n^e.
*/
void *pow(void *dta)
{
    int *n = (int *)dta;
    int *n1, *r1, t1;
    int *r = (int *)malloc(sizeof(int));
    Atrib a1;
    ++n;
    if (*n == 0) *r = 1;
    else
    {
        --n;
        n1 = (int *)malloc(sizeof(int));
        *n1 = *n;
        ++n1; 
        ++n;
        *n1 = *n - 1;
        --n1; 
        --n;
        a1.p = 0;
        a1.c = *n1;
        t1 = spawn(&a1, pow, (void *)n1);
        r1 = (int *)malloc(sizeof(int));
        sync(t1, (void **)&r1);
        *r = (*n) * (*r1);
        free(r1);
        free(n1);
    }
    return r;
}



