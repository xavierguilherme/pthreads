struct Atrib {
    int p; // Prioridade da tarefa
    int c; // Custo computacional
};

int start( int m );
void finish();
int spawn( struct Atrib* atrib, void *(*t) (void *), void* dta );
int sync( int tId, void** res );