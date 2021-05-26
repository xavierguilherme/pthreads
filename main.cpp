#include <stdio.h>
#include <malloc.h>
#include <iostream>
#include "exemplos.h"
#include "minhaBiblioteca.h"

using namespace std;

int main()
{
    int n, *r, tId, escolha, pvs, e;
    Atrib a;

    int *pow_params;

    r = (int *)malloc(sizeof(int));

    while (true) {

        cout << endl
        << "######### MENU #########\n"
        << "    1 - Fibonacci(n)\n"
        << "    2 - Fatorial(n)\n"
        << "    3 - Soma(n)\n"
        << "    4 - Pow(n, e)\n"
        << "    Outro - Sair\n"
        << "Escolha: ";
        cin >> escolha;

        if (escolha != 1 && escolha != 2 
        && escolha != 3 && escolha != 4)
            break;

        cout << endl
        << "Escolha a quantidade de pvs: ";
        cin >> pvs;

        cout << endl
        << "Escolha um valor para n: ";
        cin >> n;

        if (escolha == 4) {
            cout << endl
            << "Escolha um valor para e: ";
            cin >> e;

            pow_params = (int *)malloc(sizeof(int[2]));

            *pow_params = n;
            ++pow_params;
            *pow_params = e;
            --pow_params;
        }

        start(pvs);
        a.p = 0;
        a.c = n;

        if (escolha == 1) 
            tId = spawn(&a, fibo, &n);
        else if (escolha == 2)
            tId = spawn(&a, fat, &n);
        else if (escolha == 3)
            tId = spawn(&a, soma, &n);
        else 
            tId = spawn(&a, pow, pow_params);
                        
        sync(tId, (void **)&r);

        finish();

        if (escolha == 1)
            printf("\nFibonacci(%d) = %d\n", n, *r);
        else if (escolha == 2)
            printf("\nFatorial(%d) = %d\n", n, *r);
        else if (escolha == 3)
            printf("\nSoma(%d) = %d\n", n, *r);
        else
            printf("\nPow(%d, %d) = %d\n", n, e, *r);
            
    }

    free(r);
    free(pow_params);
    return 0;
}

