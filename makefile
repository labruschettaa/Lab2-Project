# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
SHELL=/bin/bash
CC=gcc
CFLAGS=-std=c11 -Wall -O -g
LDLIBS=-lm -pthread 

.PHONY=clean
all: archivio

archivio: archivio.o betterfunctions.o auxiliaryfunctions.o 
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

archivio.o: archivio.c betterfunctions.h auxiliaryfunctions.h 

auxiliaryfunctions.o: auxiliaryfunctions.c auxiliaryfunctions.h betterfunctions.h

betterfunctions.o: betterfunctions.c betterfunctions.h

	$(CC) $(CFLAGS) -c $< -o $@

# se si scrive solo make di default compila main.c
clean:
	-@rm -f *.o archivio