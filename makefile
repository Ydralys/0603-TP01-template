CXX=g++
CC=g++
CPPFLAGS=-g -Wall -Wconversion -std=c++11 -D_DEBUG
#-Wsign-conversion
LDLIBS=
# les règles Exemple1, Exemple2, Exemple3 sont déduites du contexte
all: Exemple1 Exemple2 Exemple3
clean:
	rm -f *.o
# dépendances
Exemple1.o: BitBase.h BitStream.h BitBlock.h
Exemple2.o: BitBase.h BitStream.h BitBlock.h
Exemple3.o: BitFloat.h
