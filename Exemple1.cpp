/// library: bitstream / exemple 1 (utilisation des Bits::Block)
/// author: pascal mignot (université de Reims)
/// version 1.2: mise-à-jour 01/2018
/// + passage à un fonctionnement en flux avec << et >> pour les lectures/ecritures dans le flux
/// + Bits::Block<x> : Block avec x bits valides fixés
/// + Bits::varBlock : Block avec un nombre de bits valides variable
/// + ajout d'une fonction Binary pour visualiser les données en binaires dans un flux.
/// + ajout de tests unitaires pour validation

#include <iostream>
#include <random>
#include <vector>
// vous devez lire l'implémentation de bitstream avant de l'utiliser
#include "BitStream.h"
using namespace std;

int main() {
	const int   val = 1350;
	int x = val;
	cout << "entier x = " << x << endl;
	cout << "en binaire: " << Bits::Binary(x) << endl;
	cout << "en binaire par paquet de 8: " << Bits::Binary(x,8) << endl;
	cout << "en binaire à partir du 24ème bit: " << Bits::Binary(x,8,24) << endl;
	cout << endl;

	// block de bits (taille fixe, support dépendant du nombre de bits à la déclaration)
	Bits::Block<12>  y(val);
	cout << "Bloc binaire de taille constante" << endl;
	cout << "Bits::Block y = " << y << endl;
	cout << "value = " << y.get() << endl;
	cout << "taille du support = " << sizeof(Bits::Block<12>::Type) << " bytes" << endl;
	cout << endl;

	// block de bits (taille variable)
	Bits::varBlock  z(12,val);
	cout << "Bloc binaire de taille variable" << endl;
	cout << "Bits::varBlock z = " << z << endl;
	cout << "value = " << z.get() << endl;
	cout << "taille du support = " << sizeof(Bits::varBlock::Type) << " bytes" << endl;
	cout << "changement du nombre de bits valides à 4 bits" << endl;
	z.set_valid(4);
	cout << "Bits::varBlock z = " << z << endl;
	cout << "value = " << z.get() << endl;
	cout << endl;

	return 0;

}
