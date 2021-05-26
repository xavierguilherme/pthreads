typedef struct {
    int p; // Prioridade da tarefa
    int c; // Custo computacional
} Atrib;

int start( int m );

void finish();

int spawn( Atrib* atrib, void *(*t) (void *), void* dta );

int sync( int tId, void** res );
