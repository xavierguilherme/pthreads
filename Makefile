# Makefile
# Linka códigos-objeto (cria executável main)
main: main.o minhaBiblioteca.o exemplos.o
	g++ -o main main.o minhaBiblioteca.o exemplos.o -lpthread

# Compila main.cpp (cria código-objeto main.o)
main.o: main.cpp minhaBiblioteca.h exemplos.h
	g++ -c -o main.o main.cpp -lpthread

# Compila exemplos.cpp (cria código-objeto exemplos.o)
exemplos.o: exemplos.cpp minhaBiblioteca.h
	g++ -c -o exemplos.o exemplos.cpp -lpthread

# Compila minhaBiblioteca.cpp (cria código-objeto minhaBiblioteca.o)
minhaBiblioteca.o: minhaBiblioteca.cpp minhaBiblioteca.h
	g++ -c -o minhaBiblioteca.o minhaBiblioteca.cpp -lpthread