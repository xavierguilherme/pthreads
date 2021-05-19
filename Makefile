# Makefile
# Linka códigos-objeto (cria executável main)
fibonacci: fibonacci.o minhaBiblioteca.o
	g++ -o fibonacci fibonacci.o minhaBiblioteca.o -lpthread

# Compila fibonacci.cpp (cria código-objeto main.o)
fibonacci.o: fibonacci.cpp minhaBiblioteca.h
	g++ -c -o fibonacci.o fibonacci.cpp -lpthread

# Compila minhaBiblioteca.cpp (cria código-objeto biblioteca.o)
minhaBiblioteca.o: minhaBiblioteca.cpp minhaBiblioteca.h
	g++ -c -o minhaBiblioteca.o minhaBiblioteca.cpp -lpthread